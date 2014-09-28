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
	for result in native_results:
		categories += ["'" + result['category'] + "'"]
		nativeScalarResults += [str(result['scalar'])]
		nativeSimdResults += [str(result['simd'])]
		html_result = find_result_in_category(html_results, result['category'])
		if html_result is not None:
			htmlScalarResults += [str(html_result['scalar'])]
			htmlSimdResults += [str(html_result['simd'])]
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
});</script>'''

html += '</body></html>'

open('results_sse1.html', 'w').write(html)