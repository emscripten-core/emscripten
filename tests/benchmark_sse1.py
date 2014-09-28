import subprocess, tempfile, os, sys, shutil, json
from subprocess import Popen, PIPE, STDOUT

__rootpath__ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
def path_from_root(*pathelems):
  return os.path.join(__rootpath__, *pathelems)
sys.path += [path_from_root('')]
import tools.shared
from tools.shared import *

temp_dir = tempfile.mkdtemp()

# Run native build
out_file = os.path.join(temp_dir, 'benchmark_sse1_native')
out = Popen([CLANG_CPP, path_from_root('tests', 'benchmark_sse1.cpp'), '-O3', '-o', out_file], stdout=PIPE, stderr=PIPE).communicate()
native_results = Popen([out_file], stdout=PIPE, stderr=PIPE).communicate()
print native_results[0]

# Run emscripten build
out_file = os.path.join(temp_dir, 'benchmark_sse1_html.html')
out = Popen([EMCC, path_from_root('tests', 'benchmark_sse1.cpp'), '-O3', '--emrun', '-s', 'TOTAL_MEMORY=536870912', '-o', out_file], stdout=PIPE, stderr=PIPE).communicate()
# We require running in FF Nightly, since no other browsers support SIMD yet.
html_results = Popen([path_from_root('emrun'), '--browser=firefox_nightly', out_file], stdout=PIPE, stderr=PIPE).communicate()
##html_results = native_results
print html_results[0]

shutil.rmtree(temp_dir)

native_results = json.loads(native_results[0])
html_results = json.loads(html_results[0])

native_workload = native_results['workload']
html_workload = html_results['workload']

html = '''<html><head></head><body>
<script src="http://ajax.googleapis.com/ajax/libs/jquery/1.8.2/jquery.min.js"></script>
<script src="http://code.highcharts.com/highcharts.js"></script>
<script src="http://code.highcharts.com/modules/exporting.js"></script>
'''

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
		else:
			htmlScalarResults += [str(-1)]
			htmlSimdResults += [str(-1)]

	chartNumber += 1
	html += '<div id="chart'+str(chartNumber)+'" style="width:100%; height:400px;"></div>'
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

html += '</body></html>'

open('results_sse1.html', 'w').write(html)