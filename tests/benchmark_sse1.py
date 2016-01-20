import subprocess, tempfile, os, sys, shutil, json
from subprocess import Popen, PIPE, STDOUT

__rootpath__ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
def path_from_root(*pathelems):
  return os.path.join(__rootpath__, *pathelems)
sys.path += [path_from_root('')]
import tools.shared
from tools.shared import *

temp_dir = tempfile.mkdtemp()

# System info
system_info = Popen([PYTHON, path_from_root('emrun'), '--system_info'], stdout=PIPE, stderr=PIPE).communicate()

# Native info
native_info = Popen(['clang', '-v'], stdout=PIPE, stderr=PIPE).communicate()

# Emscripten info
emscripten_info = Popen([PYTHON, EMCC, '-v'], stdout=PIPE, stderr=PIPE).communicate()

# Run native build
out_file = os.path.join(temp_dir, 'benchmark_sse1_native')
if WINDOWS: out_file += '.exe'
cmd = [CLANG_CPP] + get_clang_native_args() + [path_from_root('tests', 'benchmark_sse1.cpp'), '-O3', '-o', out_file]
print 'Building native version of the benchmark:'
print ' '.join(cmd)
build = Popen(cmd, env=get_clang_native_env())
out = build.communicate()
if build.returncode != 0:
    sys.exit(1)

native_results = Popen([out_file], stdout=PIPE, stderr=PIPE).communicate()
print native_results[0]

# Run emscripten build
out_file = os.path.join(temp_dir, 'benchmark_sse1_html.html')
cmd = [PYTHON, EMCC, path_from_root('tests', 'benchmark_sse1.cpp'), '-O3', '-msse', '--emrun', '-s', 'TOTAL_MEMORY=536870912', '-o', out_file]
print 'Building Emscripten version of the benchmark:'
print ' '.join(cmd)
build = Popen(cmd)
out = build.communicate()
if build.returncode != 0:
    sys.exit(1)

# The output file will have a 'almost asm' annotation, since SIMD is not yet in Nightly, so it won't even attempt to validate.
# Replace with 'use asm' to get a hint of validation errors, if those exist.
out_js_file = out_file.replace('.html', '.js')
js = open(out_js_file, 'r').read()
if 'almost asm' in js:
    print 'Replacing "almost asm" with "use asm" in generated output to attempt asm.js and detect errors with asm.js validation.'
    open(out_js_file, 'w').write(js.replace('almost asm', 'use asm'))

# Enforce asm.js validation for the output file so that we can capture any validation errors.
asmjs_validation_status = Popen([PYTHON, path_from_root('tools', 'validate_asmjs.py'), out_file], stdout=PIPE, stderr=PIPE).communicate()
asmjs_validation_status = (asmjs_validation_status[0].strip() + '\n' + asmjs_validation_status[1].strip()).strip()
if 'is not valid asm.js' in asmjs_validation_status:
    print >> sys.stderr, asmjs_validation_status
    asmjs_validation_status = '<span style="color:red;">' + asmjs_validation_status + '</span>'

browser = 'firefox_nightly'
if len(sys.argv) > 1 and sys.argv[1].startswith('--browser='):
  browser = sys.argv[1][len('--browser='):].strip()

# We require running in FF Nightly, since no other browsers support SIMD yet.
print 'Now launching Firefox to run the browser benchmark. For this to work properly, ensure the following:'
print ' - Firefox Nightly is installed.'
print ' - No version of Firefox was running beforehand (autostart conflicts with Firefox profile mechanism).'
print ' - The slow script dialog in Firefox is disabled.'
print ' - Make sure that all Firefox debugging, profiling etc. add-ons that might impact performance are disabled (Firebug, Geckoprofiler, ...).'
print ''
print 'Once the test has finished, close the browser application to continue.'
html_results = Popen([PYTHON, path_from_root('emrun'), '--browser=' + browser, out_file], stdout=PIPE, stderr=PIPE).communicate()

if not html_results or not html_results[0].strip():
    print 'Running Firefox Nightly failed! Please rerun with the command line parameter --browser=/path/to/firefox/nightly/firefox'
    sys.exit(1)

def strip_comments(text):
    return re.sub('//.*?\n|/\*.*?\*/', '', text, re.S)
benchmark_results = strip_comments(html_results[0])

# Strip out unwanted print output.
benchmark_results = benchmark_results[benchmark_results.find('{'):].strip()
if '*************************' in benchmark_results:
    benchmark_results = benchmark_results[:benchmark_results.find('*************************')].strip()

##html_results = native_results
print benchmark_results

browser_info = html_results[1]
browser_info = '<br/>'.join([line for line in browser_info.strip().split('\n') if line.startswith('User Agent')])

shutil.rmtree(temp_dir)

native_results = json.loads(native_results[0])
html_results = json.loads(benchmark_results)

native_workload = native_results['workload']
html_workload = html_results['workload']

html = '''<html><head></head><body><h1>SSE1 JavaScript Benchmark</h1>
<script src="https://ajax.googleapis.com/ajax/libs/jquery/1.8.2/jquery.min.js"></script>
<script src="https://code.highcharts.com/highcharts.js"></script>
<script src="https://code.highcharts.com/modules/exporting.js"></script><b>System Info:</b><br/>
''' + system_info[0].replace('\n', '<br/>') + '''
<b>Native Clang Compiler:</b><br/>
''' + native_info[1].replace('\n', '<br/>') + '''
<b>Emscripten Compiler:</b><br/>
''' + emscripten_info[0].replace('\n', '<br/>') + '''
<b>Browser Information:</b><br/>
''' + browser_info + '''<br/>
<b>Benchmark Build Log:</b><br/>
''' + asmjs_validation_status.replace('\n', '<br/>')

charts_native = {}
charts_html = {}
for result in native_results['results']:
	ch = result['chart']
	if not ch in charts_native: charts_native[ch] = []
	charts_native[ch] += [result]
for result in html_results['results']:
	ch = result['chart']
	if not ch in charts_html: charts_html[ch] = []
	charts_html[ch] += [result]

def find_result_in_category(results, category):
	for result in results:
		if result['category'] == category:
			return result
	return None

def format_comparison(a, b):
	if a <= b: return "<span style='color:green;font-weight:bold;'> {:10.2f}".format(b/a) + 'x FASTER</span>'
	else: return "<span style='color:red;font-weight:bold;'> {:10.2f}".format(a/b) + 'x SLOWER</span>'

chartNumber = 0

total_time_native_scalar = 0
total_time_native_simd = 0
total_time_html_scalar = 0
total_time_html_simd = 0

for chart_name in charts_native.keys():
	# Extract data for each chart.
	categories = []
	nativeScalarResults = []
	nativeSimdResults = []
	htmlScalarResults = []
	htmlSimdResults = []
	native_results = charts_native[chart_name]
	html_results = charts_html[chart_name]
	textual_results_native = '<p>'
	textual_results_html = '<p>'
	textual_results_html2 = '<p>'
	textual_results_html3 = '<p>'
	for result in native_results:
		categories += ["'" + result['category'] + "'"]
		nsc = result['scalar']
		nsi = result['simd']
		nativeScalarResults += [str(nsc)]
		nativeSimdResults += [str(nsi)]
		html_result = find_result_in_category(html_results, result['category'])
		textual_results_native += 'Native ' + result['category'] + ': ' + "{:10.4f}".format(nsc) + 'ns -> ' + "{:10.4f}".format(nsi) + 'ns. '
		textual_results_native += 'Native SSE1 is ' + format_comparison(nsi, nsc) + ' than native scalar. &nbsp; &nbsp; &nbsp; &nbsp; <br />'

		if html_result is not None:
			hsc = html_result['scalar']
			htmlScalarResults += [str(hsc)]
			hsi = html_result['simd']
			htmlSimdResults += [str(hsi)]
			textual_results_html += 'JS ' + result['category'] + ': ' + "{:10.4f}".format(hsc) + 'ns -> ' + "{:10.4f}".format(hsi) + 'ns. '
			textual_results_html += 'JS SSE1 is ' + format_comparison(hsi, hsc) + ' than JS scalar. &nbsp; &nbsp; &nbsp; &nbsp; <br />'
			textual_results_html2 += 'JS ' + result['category'] + ': JS scalar is ' + format_comparison(hsc, nsc) + ' than native scalar. &nbsp; &nbsp; &nbsp; &nbsp; <br />'
			textual_results_html3 += 'JS ' + result['category'] + ': JS SSE1 is ' + format_comparison(hsi, nsi) + ' than native SSE1. &nbsp; &nbsp; &nbsp; &nbsp; <br />'
			total_time_native_scalar += nsc
			total_time_native_simd += nsi
			total_time_html_scalar += hsc
			total_time_html_simd += hsi
		else:
			htmlScalarResults += [str(-1)]
			htmlSimdResults += [str(-1)]

	chartNumber += 1
	html += '<div id="chart'+str(chartNumber)+'" style="width:100%; height:400px; margin-top: 100px;"></div>'
	html += '''<script>$(function () { 
    $('#chart''' + str(chartNumber) + '''').highcharts({
        chart: {
            type: 'column'
        },
        title: {
            text: "'''+ chart_name + '''"
        },
        subtitle: {
            text: 'Time per operation in nanoseconds'
        },
        xAxis: {
            categories: [''' + ','.join(categories) + '''
            ]
        },
        yAxis: {
            min: 0,
            title: {
                text: 'Time (nanoseconds)'
            }
        },
        tooltip: {
            headerFormat: '<span style="font-size:10px">{point.key}</span><table>',
            pointFormat: '<tr><td style="color:{series.color};padding:0">{series.name}: </td>' +
                '<td style="padding:0"><b>{point.y:.3f} ns</b></td></tr>',
            footerFormat: '</table>',
            shared: true,
            useHTML: true
        },
        plotOptions: {
            column: {
                pointPadding: 0.2,
                borderWidth: 0
            }
        },
        series: [{
            name: 'Native scalar',
            data: [''' + ','.join(nativeScalarResults) + ''']

        }, {
            name: 'Native SSE1',
            data: [''' + ','.join(nativeSimdResults) + ''']

        }, {
            name: 'JS scalar',
            data: [''' + ','.join(htmlScalarResults) + ''']

        }, {
            name: 'JS SSE1',
            data: [''' + ','.join(htmlSimdResults) + ''']

        }]
    });
});</script>''' + '<table><tr><td>' + textual_results_native + '</td><td>' + textual_results_html + '</td></tr><tr><td>' + textual_results_html2 + '</td><td>' + textual_results_html3 + '</td></tr></table>'

# Final overall score

html += '<div id="overallscore" style="width:100%; height:400px; margin-top: 100px;"></div>'
html += '''<script>$(function () { 
    $('#overallscore').highcharts({
        chart: {
            type: 'column'
        },
        title: {
            text: "Overall Execution Time"
        },
        xAxis: {
            categories: ['Total time normalized to native']
        },
        yAxis: {
            min: 0,
            title: {
                text: 'Relative time'
            }
        },
        tooltip: {
            headerFormat: '<span style="font-size:10px">{point.key}</span><table>',
            pointFormat: '<tr><td style="color:{series.color};padding:0">{series.name}: </td>' +
                '<td style="padding:0"><b>{point.y:.3f}x</b></td></tr>',
            footerFormat: '</table>',
            shared: true,
            useHTML: true
        },
        plotOptions: {
            column: {
                pointPadding: 0.2,
                borderWidth: 0
            }
        },
        series: [{
            name: 'Native scalar',
            data: [''' + str(1.0) + ''']

        }, {
            name: 'Native SSE1',
            data: [''' + str(total_time_native_simd/total_time_native_scalar) + ''']

        }, {
            name: 'JS scalar',
            data: [''' + str(total_time_html_scalar/total_time_native_scalar) + ''']

        }, {
            name: 'JS SSE1',
            data: [''' + str(total_time_html_simd/total_time_native_scalar) + ''']

        }]
    });
});</script>'''

html += '</body></html>'

open('results_sse1.html', 'w').write(html)
print 'Wrote ' + str(len(html)) + ' bytes to file results_sse1.html.'