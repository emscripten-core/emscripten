#!/usr/bin/env python
# Copyright 2014 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import tempfile, os, sys, shutil, json, re

__rootpath__ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
def path_from_root(*pathelems):
  return os.path.join(__rootpath__, *pathelems)
sys.path += [path_from_root('')]
from tools import shared
from tools.shared import PYTHON, WINDOWS, CLANG_CXX, EMCC, PIPE, V8_ENGINE
from tools.shared import Popen


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
cmd = [CLANG_CXX] + shared.Building.get_native_building_args() + [path_from_root('tests', 'sse', 'benchmark_sse1.cpp'), '-O3', '-o', out_file]
print 'Building native version of the benchmark:'
print ' '.join(cmd)
build = Popen(cmd, env=shared.Building.get_building_env(native=True))
out = build.communicate()
if build.returncode != 0:
    sys.exit(1)

native_results = Popen([out_file], stdout=PIPE, stderr=PIPE).communicate()
print native_results[0]

# Run emscripten build
out_file = os.path.join(temp_dir, 'benchmark_sse1_html.js')
cmd = [PYTHON, EMCC, path_from_root('tests', 'sse', 'benchmark_sse1.cpp'), '-O3', '-msse', '-s', 'SIMD=1', '-s', 'TOTAL_MEMORY=536870912', '-o', out_file]
print 'Building Emscripten version of the benchmark:'
print ' '.join(cmd)
build = Popen(cmd)
out = build.communicate()
if build.returncode != 0:
    sys.exit(1)

cmd = V8_ENGINE + ['--experimental-wasm-simd', os.path.basename(out_file)]
print ' '.join(cmd)
old_dir = os.getcwd()
os.chdir(os.path.dirname(out_file))
wasm_results = Popen(cmd, stdout=PIPE, stderr=PIPE).communicate()
os.chdir(old_dir)

if not wasm_results:
    raise Exception('Unable to run benchmark in V8!')

if not wasm_results[0].strip():
    print wasm_results[1]
    sys.exit(1)

print wasm_results[0]
def strip_comments(text):
    return re.sub('//.*?\n|/\*.*?\*/', '', text, re.S)
benchmark_results = strip_comments(wasm_results[0])

# Strip out unwanted print output.
benchmark_results = benchmark_results[benchmark_results.find('{'):].strip()
if '*************************' in benchmark_results:
    benchmark_results = benchmark_results[:benchmark_results.find('*************************')].strip()

print benchmark_results

shutil.rmtree(temp_dir)

native_results = json.loads(native_results[0])
benchmark_results = benchmark_results[benchmark_results.index('{'):benchmark_results.rindex('}')+1]
wasm_results = json.loads(benchmark_results)

native_workload = native_results['workload']
html_workload = wasm_results['workload']

html = '''<html><head></head><body><h1>SSE1 JavaScript Benchmark</h1>
<script src="https://ajax.googleapis.com/ajax/libs/jquery/1.8.2/jquery.min.js"></script>
<script src="https://code.highcharts.com/highcharts.js"></script>
<script src="https://code.highcharts.com/modules/exporting.js"></script><b>System Info:</b><br/>
''' + system_info[0].replace('\n', '<br/>') + '''
<b>Native Clang Compiler:</b><br/>
''' + native_info[1].replace('\n', '<br/>') + '''
<b>Emscripten Compiler:</b><br/>
''' + emscripten_info[0].replace('\n', '<br/>')

charts_native = {}
charts_html = {}
for result in native_results['results']:
	ch = result['chart']
	if ch not in charts_native: charts_native[ch] = []
	charts_native[ch] += [result]
for result in wasm_results['results']:
	ch = result['chart']
	if ch not in charts_html: charts_html[ch] = []
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
	wasm_results = charts_html[chart_name]
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
		html_result = find_result_in_category(wasm_results, result['category'])
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
