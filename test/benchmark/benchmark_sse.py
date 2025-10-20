#!/usr/bin/env python3
# Copyright 2020 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import json
import os
import re
import subprocess
import sys

__scriptdir__ = os.path.dirname(os.path.abspath(__file__))
__testdir__ = os.path.dirname(os.path.dirname(__file__))
__rootpath__ = os.path.dirname(__testdir__)
print(__rootpath__)
sys.path.insert(0, __rootpath__)
sys.path.insert(0, __testdir__)

import clang_native
from common import EMRUN, test_file

from tools.config import V8_ENGINE
from tools.shared import CLANG_CXX, EMCC, WINDOWS, run_process

# System info
system_info = subprocess.check_output([EMRUN, '--system_info'], stderr=subprocess.STDOUT, text=True)

# Native info
native_info = subprocess.check_output(['clang', '-v'], stderr=subprocess.STDOUT, text=True)

# Emscripten info
emscripten_info = subprocess.check_output([EMCC, '-v'], stderr=subprocess.STDOUT, text=True)


def run_benchmark(benchmark_file, results_file, build_args):
    out_dir = os.path.join(__rootpath__, 'out')
    results_file = os.path.join(out_dir, results_file)
    # Run native build
    out_file = os.path.join(out_dir, 'benchmark_sse_native')
    if WINDOWS:
        out_file += '.exe'
    cmd = [CLANG_CXX] + clang_native.get_clang_native_args() + [benchmark_file, '-O3', '-o', out_file]
    print('Building native version of the benchmark:')
    print(' '.join(cmd))
    run_process(cmd, env=clang_native.get_clang_native_env())

    native_results = subprocess.check_output(out_file)
    print('native_results', native_results)

    # Run emscripten build
    out_file = os.path.join(out_dir, 'benchmark_sse_html.js')
    cmd = [EMCC, benchmark_file, '-sENVIRONMENT=web,shell,node', '-msimd128', '-O3', '-sTOTAL_MEMORY=536870912', '-o', out_file] + build_args
    print('Building Emscripten version of the benchmark:')
    print(' '.join(cmd))
    run_process(cmd)

    cmd = V8_ENGINE + [os.path.basename(out_file)]
    print(' '.join(cmd))
    wasm_results = subprocess.check_output(cmd, cwd=out_dir, text=True)

    print('wasm_results', wasm_results)

    def strip_comments(text):
        return re.sub(r'//.*?\n|/\*.*?\*/', '', text, flags=re.S) # noqa

    benchmark_results = strip_comments(wasm_results)
    print('stripped', benchmark_results)

    # Strip out unwanted print output.
    benchmark_results = benchmark_results[benchmark_results.find('{'):].strip()
    if '*************************' in benchmark_results:
        benchmark_results = benchmark_results[:benchmark_results.find('*************************')].strip()

    print(benchmark_results)

    native_results = json.loads(native_results)
    benchmark_results = benchmark_results[benchmark_results.index('{'):benchmark_results.rindex('}') + 1]
    wasm_results = json.loads(benchmark_results)

    # native_workload = native_results['workload']
    # html_workload = wasm_results['workload']

    html = '''<html><head></head><body><h1>SSE JavaScript Benchmark</h1>
    <script src="https://ajax.googleapis.com/ajax/libs/jquery/1.8.2/jquery.min.js"></script>
    <script src="https://code.highcharts.com/highcharts.js"></script>
    <script src="https://code.highcharts.com/modules/exporting.js"></script><b>System Info:</b><br/>
    ''' + system_info.replace('\n', '<br/>') + '''
    <b>Native Clang Compiler:</b><br/>
    ''' + native_info.replace('\n', '<br/>') + '''
    <b>Emscripten Compiler:</b><br/>
    ''' + emscripten_info.replace('\n', '<br/>')

    charts_native = {}
    charts_html = {}
    for result in native_results['results']:
        ch = result['chart']
        if ch not in charts_native:
            charts_native[ch] = []
        charts_native[ch] += [result]
    for result in wasm_results['results']:
        ch = result['chart']
        if ch not in charts_html:
            charts_html[ch] = []
        charts_html[ch] += [result]

    def find_result_in_category(results, category):
        for result in results:
            if result['category'] == category:
                return result
        return None

    def format_comparison(a, b):
        if a < b and a != 0:
            return "<span style='color:green;font-weight:bold;'> {:10.2f}".format(b / a) + 'x FASTER</span>'
        elif b != 0:
            return "<span style='color:red;font-weight:bold;'> {:10.2f}".format(a / b) + 'x SLOWER</span>'
        else:
            return "<span style='color:red;font-weight:bold;'> NaN </span>"

    chartNumber = 0

    total_time_native_scalar = 0
    total_time_native_simd = 0
    total_time_html_scalar = 0
    total_time_html_simd = 0

    for chart_name, chart_native_results in charts_native.items():
        # Extract data for each chart.
        categories = []
        nativeScalarResults = []
        nativeSimdResults = []
        htmlScalarResults = []
        htmlSimdResults = []
        native_results = chart_native_results
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
            textual_results_native += 'Native SSE is ' + format_comparison(nsi, nsc) + ' than native scalar. &nbsp; &nbsp; &nbsp; &nbsp; <br />'

            if html_result is not None:
                hsc = html_result['scalar']
                htmlScalarResults += [str(hsc)]
                hsi = html_result['simd']
                htmlSimdResults += [str(hsi)]
                textual_results_html += 'JS ' + result['category'] + ': ' + "{:10.4f}".format(hsc) + 'ns -> ' + "{:10.4f}".format(hsi) + 'ns. '
                textual_results_html += 'JS SSE is ' + format_comparison(hsi, hsc) + ' than JS scalar. &nbsp; &nbsp; &nbsp; &nbsp; <br />'
                textual_results_html2 += 'JS ' + result['category'] + ': JS scalar is ' + format_comparison(hsc, nsc) + ' than native scalar. &nbsp; &nbsp; &nbsp; &nbsp; <br />'
                textual_results_html3 += 'JS ' + result['category'] + ': JS SSE is ' + format_comparison(hsi, nsi) + ' than native SSE. &nbsp; &nbsp; &nbsp; &nbsp; <br />'
                total_time_native_scalar += nsc
                total_time_native_simd += nsi
                total_time_html_scalar += hsc
                total_time_html_simd += hsi
            else:
                htmlScalarResults += [str(-1)]
                htmlSimdResults += [str(-1)]

        chartNumber += 1
        html += '<div id="chart' + str(chartNumber) + '" style="width:100%; height:400px; margin-top: 100px;"></div>'
        html += '''<script>$(function () {
        $('#chart''' + str(chartNumber) + '''').highcharts({
            chart: {
                type: 'column'
            },
            title: {
                text: "''' + chart_name + '''"
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
                name: 'Native SSE',
                data: [''' + ','.join(nativeSimdResults) + ''']

            }, {
                name: 'JS scalar',
                data: [''' + ','.join(htmlScalarResults) + ''']

            }, {
                name: 'JS SSE',
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
                name: 'Native SSE',
                data: [''' + (str(total_time_native_simd / total_time_native_scalar) if total_time_native_scalar != 0 else 'N/A') + ''']

            }, {
                name: 'JS scalar',
                data: [''' + (str(total_time_html_scalar / total_time_native_scalar) if total_time_native_scalar != 0 else 'N/A') + ''']

            }, {
                name: 'JS SSE',
                data: [''' + (str(total_time_html_simd / total_time_native_scalar) if total_time_native_scalar != 0 else 'N/A') + ''']

            }]
        });
    });</script>'''

    html += '</body></html>'

    open(results_file, 'w').write(html)
    print('Wrote ' + str(len(html)) + ' bytes to file ' + results_file + '.')


if __name__ == '__main__':
    suite = sys.argv[1].lower() if len(sys.argv) == 2 else None
    if suite in ['sse', 'sse1']:
        run_benchmark(test_file('benchmark/benchmark_sse1.cpp'), 'results_sse1.html', ['-msse'])
    elif suite == 'sse2':
        run_benchmark(test_file('benchmark/benchmark_sse2.cpp'), 'results_sse2.html', ['-msse2'])
    elif suite == 'sse3':
        run_benchmark(test_file('benchmark/benchmark_sse3.cpp'), 'results_sse3.html', ['-msse3'])
    elif suite == 'ssse3':
        run_benchmark(test_file('benchmark/benchmark_ssse3.cpp'), 'results_ssse3.html', ['-mssse3'])
    else:
        raise Exception('Usage: python test/benchmark/benchmark_sse.py sse1|sse2|sse3')
