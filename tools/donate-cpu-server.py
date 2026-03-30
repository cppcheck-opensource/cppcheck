#!/usr/bin/env python3

# Server for 'donate-cpu.py'
# Runs only under Python 3.

import collections
import glob
import json
import os
import socket
import re
import datetime
import time
import traceback
from threading import Thread
import sys
import urllib.request
import urllib.parse
import urllib.error
import logging
import logging.handlers
import operator
import html as html_lib
from urllib.parse import urlparse

# Version scheme (MAJOR.MINOR.PATCH) should orientate on "Semantic Versioning" https://semver.org/
# Every change in this script should result in increasing the version number accordingly (exceptions may be cosmetic
# changes)
SERVER_VERSION = "1.3.68"

# TODO: fetch from GitHub tags
OLD_VERSION = '2.20.0'

HEAD_MARKER = 'head results:'
INFO_MARKER = 'info messages:'


# Set up logging
logger = logging.getLogger()
logger.setLevel(logging.INFO)
# Logging to console
handler_stream = logging.StreamHandler()
logger.addHandler(handler_stream)
# Log errors to a rotating file
logfile = sys.path[0]
if logfile:
    logfile += '/'
logfile += 'donate-cpu-server.log'
handler_file = logging.handlers.RotatingFileHandler(filename=logfile, maxBytes=100*1024, backupCount=1)
handler_file.setFormatter(logging.Formatter('%(asctime)s %(message)s'))
handler_file.setLevel(logging.ERROR)
logger.addHandler(handler_file)


def print_ts(msg) -> None:
    dt = datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S.%f')
    print(f'[{dt}] {msg}')


# Set up an exception hook for all uncaught exceptions so they can be logged
def handle_uncaught_exception(exc_type, exc_value, exc_traceback):
    if issubclass(exc_type, KeyboardInterrupt):
        sys.__excepthook__(exc_type, exc_value, exc_traceback)
        return

    logging.error("Uncaught exception", exc_info=(exc_type, exc_value, exc_traceback))


sys.excepthook = handle_uncaught_exception


def strDateTime() -> str:
    return datetime.datetime.now().strftime('%Y-%m-%d %H:%M')


def dateTimeFromStr(datestr: str) -> datetime.datetime:
    return datetime.datetime.strptime(datestr, '%Y-%m-%d %H:%M')


def pkg_from_file(filename: str) -> str:
    return filename[filename.rfind('/')+1:]


def overviewReport() -> str:
    html = '<!DOCTYPE html>\n'
    html += '<html><head><title>daca@home</title></head><body>\n'
    html += '<h1>daca@home</h1>\n'
    html += '<a href="crash.html">Crash report</a> - <a href="crash.html?pkgs=1">packages.txt</a><br>\n'
    html += '<a href="timeout.html">Timeout report</a><br>\n'
    html += '<a href="stale.html">Stale report</a><br>\n'
    html += '<a href="diff.html">Diff report</a><br>\n'
    html += '<a href="head.html">HEAD report</a><br>\n'
    html += '<a href="headinfo.html">HEAD (information) report</a><br>\n'
    html += '<a href="latest.html">Latest results</a><br>\n'
    html += '<a href="time_lt.html">Time report (improved)</a><br>\n'
    html += '<a href="time_gt.html">Time report (regressed)</a> - <a href="time_gt.html?pkgs=1">packages.txt</a><br>\n'
    html += '<a href="time_slow.html">Time report (slowest)</a><br>\n'
    html += '<br>\n'
    html += '--check-library:<br>\n'
    html += '<a href="check_library_function_report.html">checkLibraryFunction report</a><br>\n'
    html += '<a href="check_library_noreturn_report.html">checkLibraryNoReturn report</a><br>\n'
    html += '<a href="check_library_use_ignore_report.html">checkLibraryUseIgnore report</a><br>\n'
    html += '<a href="check_library_check_type_report.html">checkLibraryCheckType report</a><br>\n'
    html += '<br>\n'
    html += 'Debug warnings:<br>\n'
    html += '<a href="head-debug">debug</a><br>\n'
    html += '<a href="head-varid0">varid0</a><br>\n'
    html += '<a href="head-valueType">valueType</a><br>\n'
    html += '<a href="head-noparamend">noparamend</a><br>\n'
    html += '<a href="head-simplifyTypedef">simplifyTypedef</a><br>\n'
    html += '<a href="head-simplifyUsingUnmatchedBodyEnd">simplifyUsingUnmatchedBodyEnd</a><br>\n'
    html += '<a href="head-simplifyUsing">simplifyUsing</a><br>\n'
    html += '<a href="head-valueFlowMaxIterations">valueFlowMaxIterations</a><br>\n'
    html += '<a href="head-templateInstantiation">templateInstantiation</a><br>\n'
    #html += '<a href="head-autoNoType">autoNoType</a><br>\n'
    #html += '<a href="head-valueFlowBailout">valueFlowBailout</a><br>\n'
    #html += '<a href="head-bailoutUninitVar">bailoutUninitVar</a><br>\n'
    #html += '<a href="head-symbolDatabaseWarning">symbolDatabaseWarning</a><br>\n'
    html += '<br>\n'
    html += 'Custom reports:<br>\n'
    html += '<a href="value_flow_bailout_incomplete_var.html">valueFlowBailoutIncompleteVar report</a><br>\n'
    html += '<a href="unknown_macro.html">unknownMacro report</a><br>\n'
    html += '<br>\n'
    html += 'Important errors:<br>\n'
    html += '<a href="head-cppcheckError">cppcheckError</a><br>\n'
    html += '<a href="head-internalError">internalError</a><br>\n'
    html += '<a href="head-internalAstError">internalAstError</a><br>\n'
    html += '<a href="head-syntaxError">syntaxError</a><br>\n'
    html += '<a href="head-DacaWrongData">DacaWrongData</a><br>\n'
    html += '<a href="head-dacaWrongSplitTemplateRightAngleBrackets">dacaWrongSplitTemplateRightAngleBrackets</a><br>\n'
    html += '<br>\n'
    html += '<a href="clients.html">clients</a><br>\n'
    html += '<br>\n'
    html += 'version ' + SERVER_VERSION + '\n'
    html += '</body></html>'
    return html


def fmt(a: str, b: str, c: str = None, d: str = None, e: str = None, link: bool = True, column_width=None) -> str:
    if column_width is None:
        column_width = [40, 10, 5, 7, 7, 8]
    ret = a
    while len(ret) < column_width[0]:
        ret += ' '
    if len(ret) == column_width[0]:
        ret += ' ' + b[:10]
    while len(ret) < (column_width[0] + 1 + column_width[1]):
        ret += ' '
    ret += ' '
    if len(b) > 10:
        ret += b[-5:].rjust(column_width[2]) + ' '
    if c is not None:
        ret += c.rjust(column_width[3]) + ' '
    if d is not None:
        ret += d.rjust(column_width[4]) + ' '
    if e is not None:
        ret += e.rjust(column_width[5])
    if link:
        pos = ret.find(' ')
        ret = '<a href="' + a + '">' + a + '</a>' + ret[pos:]
    return ret


def latestReport(latestResults: list) -> str:
    html = '<!DOCTYPE html>\n'
    html += '<html><head><title>Latest daca@home results</title></head><body>\n'
    html += '<h1>Latest daca@home results</h1>\n'
    html += '<pre>\n<b>' + fmt('Package', 'Date       Time', OLD_VERSION, 'Head', 'Diff', link=False) + '</b>\n'

    # Write report for latest results
    for filename in latestResults:
        if not os.path.isfile(filename):
            continue
        package = pkg_from_file(filename)
        current_year = datetime.date.today().year

        datestr = None
        count = ['0', '0']
        lost = 0
        added = 0
        for line in open(filename):
            line = line.strip()
            if datestr is None and line.startswith(str(current_year) + '-') or line.startswith(str(current_year - 1) + '-'):
                datestr = line
            #elif line.startswith('cppcheck:'):
            #    cppcheck = line[9:]
            elif line.startswith('count: '):
                count = line.split(' ')[1:]
            elif line.startswith('head ') and not line.startswith('head results:'):
                added += 1
            elif line.startswith(OLD_VERSION + ' '):
                lost += 1
        diff = ''
        if lost > 0:
            diff += '-' + str(lost)
        if added > 0:
            diff += '+' + str(added)
        html += fmt(package, datestr, count[1], count[0], diff) + '\n'

    html += '</pre></body></html>\n'
    return html


def crashReport(results_path: str, query_params: dict):
    pkgs = '' if query_params.get('pkgs') == '1' else None

    html = '<!DOCTYPE html>\n'
    html += '<html><head><title>Crash report</title></head><body>\n'
    html += '<h1>Crash report</h1>\n'
    html += '<pre>\n'
    html += '<b>' + fmt('Package', 'Date       Time', OLD_VERSION, 'Head', link=False) + '</b>\n'
    current_year = datetime.date.today().year
    stack_traces = {}
    for filename in sorted(glob.glob(os.path.expanduser(results_path + '/*'))):
        if not os.path.isfile(filename) or filename.endswith('.diff'):
            continue
        with open(filename) as file_:
            datestr = None
            package_url = None
            for line in file_:
                line = line.strip()
                if line.startswith('cppcheck: '):
                    if OLD_VERSION not in line:
                        # Package results seem to be too old, skip
                        break
                    # Current package, parse on
                    continue
                if datestr is None and line.startswith(str(current_year) + '-') or line.startswith(str(current_year - 1) + '-'):
                    datestr = line
                elif pkgs is not None and package_url is None and line.startswith('ftp://'):
                    package_url = line
                elif line.startswith('count:'):
                    if line.find('Crash') < 0:
                        break
                    package = pkg_from_file(filename)
                    counts = line.split(' ')
                    c_version = ''
                    if counts[2] == 'Crash!':
                        c_version = 'Crash'
                    c_head = ''
                    if counts[1] == 'Crash!':
                        c_head = 'Crash'
                    html += fmt(package, datestr, c_version, c_head) + '\n'
                    if c_head != 'Crash':
                        break
                    if package_url is not None:
                        pkgs += f'{package_url}\n'
                elif line.find(' received signal ') != -1:
                    crash_line = next(file_, '').strip()
                    location_index = crash_line.rfind(' at ')
                    if location_index > 0:
                        code_line = next(file_, '').strip()
                    else:
                        code_line = ''
                    stack_trace = []
                    while True:
                        l = next(file_, '')
                        if not l.strip():
                            break
                        # #0  0x00007ffff71cbf67 in raise () from /lib64/libc.so.6
                        m = re.search(r'(?P<number>#\d+) .* in (?P<function>.+)\(.*\) from (?P<binary>.*)$', l)
                        if m:
                            #print('0 - {} - {} - {}'.format(m.group('number'), m.group('function'), m.group('binary')))
                            stack_trace.append(m.group('number') + ' ' + m.group('function') + '(...) from ' + m.group('binary'))
                            continue
                        # #11 0x00000000006f2414 in valueFlowNumber (tokenlist=tokenlist@entry=0x7fffffffc610) at build/valueflow.cpp:2503
                        m = re.search(r'(?P<number>#\d+) .* in (?P<function>.+?) \(.*\) at (?P<location>.*)$', l)
                        if m:
                            #print('1 - {} - {} - {}'.format(m.group('number'), m.group('function'), m.group('location')))
                            stack_trace.append(m.group('number') + ' ' + m.group('function') + '(...) at ' + m.group('location'))
                            continue
                        # #18 ForwardTraversal::updateRecursive (this=0x7fffffffb3c0, tok=0x14668a0) at build/forwardanalyzer.cpp:415
                        m = re.search(r'(?P<number>#\d+) (?P<function>.+)\(.*\) at (?P<location>.*)$', l)
                        if m:
                            #print('2 - {} - {} - {}'.format(m.group('number'), m.group('function'), m.group('location')))
                            stack_trace.append(m.group('number') + ' ' + m.group('function') + '(...) at ' + m.group('location'))
                            continue

                        print_ts(f'{package} - unmatched stack frame - {l}')
                        break
                    key = hash(' '.join(stack_trace))

                    if key in stack_traces:
                        stack_traces[key]['code_line'] = code_line
                        stack_traces[key]['stack_trace'] = stack_trace
                        stack_traces[key]['n'] += 1
                        stack_traces[key]['packages'].append(package)
                    else:
                        stack_traces[key] = {'stack_trace': stack_trace, 'n': 1, 'code_line': code_line, 'packages': [package], 'crash_line': crash_line}
                    break

    html += '</pre>\n'
    html += '<pre>\n'
    html += '<b>Stack traces</b>\n'
    for stack_trace in sorted(list(stack_traces.values()), key=lambda x: x['n'], reverse=True):
        html += 'Packages: ' + ' '.join(['<a href="' + p + '">' + p + '</a>' for p in stack_trace['packages']]) + '\n'
        html += html_lib.escape(stack_trace['crash_line']) + '\n'
        html += html_lib.escape(stack_trace['code_line']) + '\n'
        html += html_lib.escape('\n'.join(stack_trace['stack_trace'])) + '\n\n'
    html += '</pre>\n'

    html += '</body></html>\n'
    if pkgs is not None:
        return pkgs, 'text/plain'
    return html, 'text/html'


def timeoutReport(results_path: str) -> str:
    html = '<!DOCTYPE html>\n'
    html += '<html><head><title>Timeout report</title></head><body>\n'
    html += '<h1>Timeout report</h1>\n'
    html += '<pre>\n'
    html += '<b>' + fmt('Package', 'Date       Time', OLD_VERSION, 'Head', link=False) + '</b>\n'
    current_year = datetime.date.today().year
    for filename in sorted(glob.glob(os.path.expanduser(results_path + '/*'))):
        if not os.path.isfile(filename) or filename.endswith('.diff'):
            continue
        with open(filename) as file_:
            datestr = None
            for line in file_:
                line = line.strip()
                if line.startswith('cppcheck: '):
                    if OLD_VERSION not in line:
                        # Package results seem to be too old, skip
                        break
                    # Current package, parse on
                    continue
                if datestr is None and line.startswith(str(current_year) + '-') or line.startswith(str(current_year - 1) + '-'):
                    datestr = line
                elif line.startswith('count:'):
                    if line.find('TO!') < 0:
                        break
                    package = pkg_from_file(filename)
                    counts = line.split(' ')
                    c2 = ''
                    if counts[2] == 'TO!':
                        c2 = 'Timeout'
                    c1 = ''
                    if counts[1] == 'TO!':
                        c1 = 'Timeout'
                    html += fmt(package, datestr, c2, c1) + '\n'
                    break

    html += '</pre>\n'
    html += '</body></html>\n'
    return html


def staleReport(results_path: str, query_params: dict) -> str:
    thresh_d = query_params.get('days')
    if thresh_d is None:
        thresh_d = 30
    else:
        thresh_d = int(thresh_d)

    html = '<!DOCTYPE html>\n'
    html += '<html><head><title>Stale report</title></head><body>\n'
    html += '<h1>Stale report</h1>\n'
    html += '<pre>\n'
    html += '<b>' + fmt('Package', 'Date       Time', link=False) + '</b>\n'
    for filename in sorted(glob.glob(os.path.expanduser(results_path + '/*'))):
        if filename.endswith('.diff') or not os.path.isfile(filename):
            continue
        with open(filename) as f:
            # first line is datetime string
            datestr = f.readline().strip()
            try:
                dt = dateTimeFromStr(datestr)
                diff = datetime.datetime.now() - dt
            except:
                # there might be very outdated files which still might have an invalid timestamp
                diff = datetime.timedelta(days=thresh_d)
            if diff.days >= thresh_d:
                package = pkg_from_file(filename)
                html += fmt(package, datestr) + '\n'
    html += '</pre>\n'

    html += '</body></html>\n'
    return html


def diffReportFromDict(out: dict, today: str) -> str:
    html = '<pre>\n'
    html += '<b>MessageID                           ' + OLD_VERSION + '    Head</b>\n'
    sum0 = 0
    sum1 = 0
    for messageId in sorted(out.keys()):
        line = messageId + ' '
        counts = out[messageId]
        sum0 += counts[0]
        sum1 += counts[1]
        if counts[0] > 0:
            c = str(counts[0])
            while len(line) < 40 - len(c):
                line += ' '
            line += c + ' '
        if counts[1] > 0:
            c = str(counts[1])
            while len(line) < 48 - len(c):
                line += ' '
            line += c
        line = '<a href="diff' + today + '-' + messageId + '">' + messageId + '</a>' + line[line.find(' '):]
        html += line + '\n'

    # Sum
    html += '================================================\n'
    line = ''
    while len(line) < 40 - len(str(sum0)):
        line += ' '
    line += str(sum0) + ' '
    while len(line) < 48 - len(str(sum1)):
        line += ' '
    line += str(sum1)
    html += line + '\n'
    html += '</pre>\n'

    return html


def diffReport(resultsPath: str) -> str:
    out = {}
    outToday = {}
    today = strDateTime()[:10]

    for filename in sorted(glob.glob(resultsPath + '/*.diff')):
        if not os.path.isfile(filename):
            continue
        with open(filename) as f:
            data = json.loads(f.read())
        uploadedToday = data['date'] == today
        for messageId in data['sums']:
            sums = data['sums'][messageId]
            if OLD_VERSION not in sums:
                break
            if messageId not in out:
                out[messageId] = [0, 0]
            out[messageId][0] += sums[OLD_VERSION]
            out[messageId][1] += sums['head']
            if uploadedToday:
                if messageId not in outToday:
                    outToday[messageId] = [0, 0]
                outToday[messageId][0] += sums[OLD_VERSION]
                outToday[messageId][1] += sums['head']

    html = '<!DOCTYPE html>\n'
    html += '<html><head><title>Diff report</title></head><body>\n'
    html += '<h1>Diff report</h1>\n'
    html += '<h2>Uploaded today</h2>'
    html += diffReportFromDict(outToday, 'today')
    html += '<h2>All</h2>'
    html += diffReportFromDict(out, '')

    return html


def generate_package_diff_statistics(filename: str) -> None:
    is_diff = False

    sums = {}

    for line in open(filename):
        line = line.strip()
        if line == 'diff:':
            is_diff = True
            continue
        if not is_diff:
            continue
        if not line.endswith(']'):
            continue

        if line.startswith(OLD_VERSION + ' '):
            version = OLD_VERSION
        elif line.startswith('head '):
            version = 'head'
        else:
            continue

        messageId = line[line.rfind('[')+1:len(line)-1]

        if messageId not in sums:
            sums[messageId] = {OLD_VERSION: 0, 'head': 0}

        sums[messageId][version] += 1

    output = {'date': strDateTime()[:10], 'sums': sums}

    filename_diff = filename + '.diff'
    if sums:
        with open(filename_diff, 'w') as f:
            f.write(json.dumps(output))
    elif os.path.isfile(filename_diff):
        os.remove(filename_diff)


def diffMessageIdReport(resultPath: str, messageId: str) -> str:
    text = messageId + '\n'
    e = '[' + messageId + ']\n'
    for filename in sorted(glob.glob(resultPath + '/*.diff')):
        if not os.path.isfile(filename):
            continue
        with open(filename) as f:
            diff_stats = json.loads(f.read())
        if messageId not in diff_stats['sums']:
            continue
        if OLD_VERSION not in diff_stats['sums'][messageId]:
            continue

        url = None
        diff = False
        for line in open(filename[:-5]):
            if line.startswith('ftp://'):
                url = line
            elif line == 'diff:\n':
                diff = True
            elif not diff:
                continue
            elif line.endswith(e):
                if url:
                    text += url
                    url = None
                text += line
    return text


def diffMessageIdTodayReport(resultPath: str, messageId: str) -> str:
    text = messageId + '\n'
    e = '[' + messageId + ']\n'
    today = strDateTime()[:10]
    for filename in sorted(glob.glob(resultPath + '/*.diff')):
        if not os.path.isfile(filename):
            continue
        with open(filename) as f:
            diff_stats = json.loads(f.read())
        if messageId not in diff_stats['sums']:
            continue
        if OLD_VERSION not in diff_stats['sums'][messageId]:
            continue
        if today not in diff_stats["date"]:
            continue

        url = None
        diff = False
        firstLine = True
        for line in open(filename[:-5]):
            if firstLine:
                firstLine = False
                if not line.startswith(today):
                    break
            if line.startswith('ftp://'):
                url = line
            elif line == 'diff:\n':
                diff = True
            elif not diff:
                continue
            elif line.endswith(e):
                if url:
                    text += url
                    url = None
                text += line
    return text


def summaryReportFromDict(out: dict, prefix: str, today: str) -> str:
    html = '<pre>\n'
    html += '<b>MessageID                                  Count</b>\n'
    sumTotal = 0
    for messageId in sorted(out.keys()):
        line = messageId + ' '
        counts = out[messageId]
        sumTotal += counts
        if counts > 0:
            c = str(counts)
            while len(line) < 48 - len(c):
                line += ' '
            line += c + ' '
        line = '<a href="' + prefix + today + '-' + messageId + '">' + messageId + '</a>' + line[line.find(' '):]
        html += line + '\n'

    # Sum
    html += '================================================\n'
    line = ''
    while len(line) < 48 - len(str(sumTotal)):
        line += ' '
    line += str(sumTotal) + ' '
    html += line + '\n'
    html += '</pre>\n'

    return html


def summaryReport(resultsPath: str, name: str, prefix: str, marker: str) -> str:
    out = {}
    outToday = {}
    today = strDateTime()[:10]

    for filename in sorted(glob.glob(resultsPath + '/*')):
        if not os.path.isfile(filename) or filename.endswith('.diff'):
            continue
        uploadedToday = False
        firstLine = True
        inResults = False
        for line in open(filename):
            if firstLine:
                if line.startswith(today):
                    uploadedToday = True
                firstLine = False
                continue
            line = line.strip()
            if line.startswith('cppcheck: '):
                if OLD_VERSION not in line:
                    # Package results seem to be too old, skip
                    break
                # Current package, parse on
                continue
            if line.startswith(marker):
                inResults = True
                continue
            if not inResults:
                continue
            if line.startswith('diff:'):
                break
            if not line.endswith(']'):
                continue
            if ': note: ' in line:
                # notes normally do not contain message ids but can end with ']'
                continue
            message_id_start_pos = line.rfind('[')
            if message_id_start_pos <= 0:
                continue
            messageId = line[message_id_start_pos+1:len(line)-1]
            if ' ' in messageId:
                # skip invalid messageIds
                continue

            if messageId not in out:
                out[messageId] = 0
            out[messageId] += 1
            if uploadedToday:
                if messageId not in outToday:
                    outToday[messageId] = 0
                outToday[messageId] += 1

    html = '<!DOCTYPE html>\n'
    html += f'<html><head><title>{name} report</title></head><body>\n'
    html += '<h1>HEAD report</h1>\n'
    html += '<h2>Uploaded today</h2>'
    html += summaryReportFromDict(outToday, prefix, 'today')
    html += '<h2>All</h2>'
    html += summaryReportFromDict(out, prefix, '')

    return html


def headReport(resultsPath: str) -> str:
    return summaryReport(resultsPath, 'HEAD', 'head', HEAD_MARKER)


def infoReport(resultsPath: str) -> str:
    return summaryReport(resultsPath, 'HEAD (information)', 'headinfo', INFO_MARKER)


def messageIdReport(resultPath: str, marker: str, messageId: str, query_params: dict) -> str:
    pkgs = '' if query_params.get('pkgs') == '1' else None
    text = messageId + '\n'
    e = '[' + messageId + ']\n'
    for filename in sorted(glob.glob(resultPath + '/*')):
        if not os.path.isfile(filename) or filename.endswith('.diff'):
            continue
        url = None
        inResults = False
        for line in open(filename):
            if line.startswith('cppcheck: '):
                if OLD_VERSION not in line:
                    # Package results seem to be too old, skip
                    break
                # Current package, parse on
                continue
            if line.startswith('ftp://'):
                url = line
                continue
            if not inResults:
                if line.startswith(marker):
                    inResults = True
                continue
            if line.startswith('diff:'):
                break
            if line.endswith(e):
                if url:
                    text += url
                    if pkgs is not None:
                        pkgs += url
                    url = None
                text += line
    if pkgs is not None:
        return pkgs
    return text


def headMessageIdReport(resultPath: str, messageId: str, query_params: dict) -> str:
    return messageIdReport(resultPath, HEAD_MARKER, messageId, query_params)


def infoMessageIdReport(resultPath: str, messageId: str, query_params: dict) -> str:
    return messageIdReport(resultPath, INFO_MARKER, messageId, query_params)


def messageIdTodayReport(resultPath: str, messageId: str, marker: str) -> str:
    text = messageId + '\n'
    e = '[' + messageId + ']\n'
    today = strDateTime()[:10]
    for filename in sorted(glob.glob(resultPath + '/*')):
        if not os.path.isfile(filename) or filename.endswith('.diff'):
            continue
        url = None
        inResults = False
        firstLine = True
        for line in open(filename):
            if firstLine:
                firstLine = False
                if not line.startswith(today):
                    break
            if line.startswith('ftp://'):
                url = line
                continue
            if not inResults:
                if line.startswith(marker):
                    inResults = True
                continue
            if line.startswith('diff:'):
                break
            if line.endswith(e):
                if url:
                    text += url
                    url = None
                text += line
    return text


def headMessageIdTodayReport(resultPath: str, messageId: str) -> str:
    return messageIdTodayReport(resultPath, messageId, HEAD_MARKER)


def infoMessageIdTodayReport(resultPath: str, messageId: str) -> str:
    return messageIdTodayReport(resultPath, messageId, INFO_MARKER)


# TODO: needs to dinicate that it returns 'tuple[str, str]' but that isn't supported until Python 3.9
def timeReport(resultPath: str, show_gt: bool, query_params: dict):
    # no need for package report support in "improved" report
    pkgs = '' if show_gt and query_params and query_params.get('pkgs') == '1' else None
    factor = float(query_params.get('factor')) if query_params and 'factor' in query_params else 2.0
    if not show_gt:
        factor = 1.0 / factor

    title = 'Time report ({})'.format('regressed' if show_gt else 'improved')
    html = '<!DOCTYPE html>\n'
    html += f'<html><head><title>{title}</title></head><body>\n'
    html += f'<h1>{title}</h1>\n'
    html += '<pre>\n'
    column_width = [40, 10, 10, 10, 10, 10]
    html += '<b>'
    html += fmt('Package', 'Date       Time', OLD_VERSION, 'Head', 'Factor', link=False, column_width=column_width)
    html += '</b>\n'

    current_year = datetime.date.today().year

    data = {}

    total_time_base = 0.0
    total_time_head = 0.0
    for filename in glob.glob(resultPath + '/*'):
        if not os.path.isfile(filename) or filename.endswith('.diff'):
            continue
        datestr = None
        package_url = None
        for line in open(filename):
            line = line.strip()
            if line.startswith('cppcheck: '):
                if OLD_VERSION not in line:
                    # Package results seem to be too old, skip
                    break
                # Current package, parse on
                continue
            if datestr is None and line.startswith(str(current_year) + '-') or line.startswith(str(current_year - 1) + '-'):
                datestr = line
                continue
            if pkgs is not None and package_url is None and line.startswith('ftp://'):
                package_url = line
            if not line.startswith('elapsed-time:'):
                continue
            split_line = line.split()
            time_base = float(split_line[2])
            time_head = float(split_line[1])
            if time_base < 0.0 or time_head < 0.0:
                # ignore results with crashes / errors for the time report
                break
            if time_base == 0.0 and time_head == 0.0:
                # no difference possible
                break
            total_time_base += time_base
            total_time_head += time_head
            if time_base == time_head:
                # no difference
                break
            if time_base > 0.0 and time_head > 0.0:
                time_factor = time_head / time_base
            elif time_base == 0.0:
                # the smallest possible value is 0.1 so treat that as an increase of 100%
                # on top of the existing 100% (treating the base 0.0 as such).
                time_factor = 1.0 + (time_head * 10)
            else:
                time_factor = 0.0
            suspicious_time_difference = False
            if show_gt and time_factor > factor:
                suspicious_time_difference = True
            elif not show_gt and time_factor < factor:
                suspicious_time_difference = True
            if suspicious_time_difference:
                pkg_name = filename[len(resultPath)+1:]
                data[pkg_name] = (datestr, split_line[2], split_line[1], time_factor)

                if package_url is not None:
                    pkgs += f'{package_url}\n'
            break

    sorted_data = sorted(data.items(), key=lambda kv: kv[1][3], reverse=show_gt)
    sorted_dict = collections.OrderedDict(sorted_data)
    for key in sorted_dict:
        html += fmt(key, sorted_dict[key][0], sorted_dict[key][1], sorted_dict[key][2], f'{sorted_dict[key][3]:.2f}',
                    column_width=column_width) + '\n'

    html += '\n'
    html += '(listed above are all suspicious timings with a factor '
    html += '&gt;' if show_gt else '&lt;'
    html += ' {}'.format(format(factor, '.2f'))
    html += ')\n'
    html += '\n'
    if total_time_base > 0.0:
        total_time_factor = total_time_head / total_time_base
    else:
        total_time_factor = 0.0
    html += 'Time for all packages (not just the ones listed above):\n'
    html += fmt('Total time:',
            '',
            f'{total_time_base:.1f}',
            f'{total_time_head:.1f}',
            f'{total_time_factor:.2f}', link=False, column_width=column_width)

    html += '\n'
    html += '</pre>\n'
    html += '</body></html>\n'

    if pkgs is not None:
        return pkgs, 'text/plain'
    return html, 'text/html'


def timeReportSlow(resultPath: str) -> str:
    title = 'Time report (slowest)'
    html = '<!DOCTYPE html>\n'
    html += f'<html><head><title>{title}</title></head><body>\n'
    html += f'<h1>{title}</h1>\n'
    html += '<pre>\n'
    html += '<b>'
    html += fmt('Package', 'Date       Time', OLD_VERSION, 'Head', link=False)
    html += '</b>\n'

    current_year = datetime.date.today().year

    data = {}

    for filename in glob.glob(resultPath + '/*'):
        if not os.path.isfile(filename) or filename.endswith('.diff'):
            continue
        datestr = None
        for line in open(filename):
            line = line.strip()
            if line.startswith('cppcheck: '):
                if OLD_VERSION not in line:
                    # Package results seem to be too old, skip
                    break
                # Current package, parse on
                continue
            if datestr is None and line.startswith(str(current_year) + '-') or line.startswith(str(current_year - 1) + '-'):
                datestr = line
                continue
            if line.startswith('count:'):
                count_head = line.split()[1]
                if count_head == 'TO!':
                    # ignore results with timeouts
                    break
                continue
            if not line.startswith('elapsed-time:'):
                continue
            split_line = line.split()
            time_base = float(split_line[2])
            time_head = float(split_line[1])
            if time_base < 0.0 or time_head < 0.0:
                # ignore results with crashes / errors
                break
            pkg_name = filename[len(resultPath)+1:]
            data[pkg_name] = (datestr, split_line[2], split_line[1], time_head)
            break

        sorted_data = sorted(data.items(), key=lambda kv: kv[1][3])
        if len(data) > 100:
            first_key, _ = sorted_data[0]
            # remove the entry with the lowest run-time
            del data[first_key]

    sorted_data = sorted(data.items(), key=lambda kv: kv[1][3], reverse=True)
    sorted_dict = collections.OrderedDict(sorted_data)
    for key in sorted_dict:
        html += fmt(key, sorted_dict[key][0], sorted_dict[key][1], sorted_dict[key][2]) + '\n'
    html += '</pre>\n'
    html += '</body></html>\n'

    return html


def check_library_report(result_path: str, message_id: str) -> str:
    if message_id not in ('checkLibraryNoReturn', 'checkLibraryFunction', 'checkLibraryUseIgnore', 'checkLibraryCheckType', 'valueFlowBailoutIncompleteVar', 'unknownMacro'):
        error_message = 'Invalid value ' + message_id + ' for message_id parameter.'
        print_ts(error_message)
        return error_message

    if message_id == 'unknownMacro':
        metric = 'macros'
        m_column = 'macro'
        metric_link = 'unknown_macro'
        start_marker = HEAD_MARKER
    elif message_id == 'valueFlowBailoutIncompleteVar':
        metric = 'variables'
        m_column = 'Variable'
        metric_link = 'incomplete_var'
        start_marker = HEAD_MARKER
    elif message_id == 'checkLibraryCheckType':
        metric = 'types'
        m_column = 'Type'
        metric_link = 'check_library'
        start_marker = INFO_MARKER
    else:
        metric = 'functions'
        m_column = 'Function'
        metric_link = 'check_library'
        start_marker = INFO_MARKER

    functions_shown_max = 5000
    html = '<!DOCTYPE html>\n'
    html += '<html><head><title>' + message_id + ' report</title></head><body>\n'
    html += '<h1>' + message_id + ' report</h1>\n'
    html += 'Top ' + str(functions_shown_max) + ' ' + metric + ' are shown.'
    html += '<pre>\n'
    column_widths = [10, 100]
    html += '<b>'
    html += 'Count'.rjust(column_widths[0]) + ' ' + m_column
    html += '</b>\n'

    function_counts = {}
    for filename in glob.glob(result_path + '/*'):
        if not os.path.isfile(filename) or filename.endswith('.diff'):
            continue
        in_results = False
        for line in open(filename):
            if line.startswith('cppcheck: '):
                if OLD_VERSION not in line:
                    # Package results seem to be too old, skip
                    break
                # Current package, parse on
                continue
            if not in_results:
                if line.startswith(start_marker):
                    in_results = True
                continue
            if line.startswith('diff:'):
                break
            if line.endswith('[' + message_id + ']\n'):
                if message_id == 'unknownMacro':
                    marker = 'required. If '
                    function_name = line[(line.find(marker) + len(marker)):line.rfind('is a macro') - 1]
                elif message_id == 'valueFlowBailoutIncompleteVar':
                    marker = 'incomplete variable '
                    function_name = line[(line.find(marker) + len(marker)):line.rfind('[') - 1]
                elif message_id == 'checkLibraryFunction':
                    marker = 'for function '
                    function_name = line[(line.find(marker) + len(marker)):line.rfind('[') - 1]
                elif message_id == 'checkLibraryCheckType':
                    marker = 'configuration for '
                    function_name = line[(line.find(marker) + len(marker)):line.rfind('[') - 1]
                else:
                    marker = ': Function '
                    function_name = line[(line.find(marker) + len(marker)):line.rfind('should have') - 1]
                function_counts[function_name] = function_counts.setdefault(function_name, 0) + 1

    function_details_list = []
    for function_name, count in sorted(list(function_counts.items()), key=operator.itemgetter(1), reverse=True):
        if len(function_details_list) >= functions_shown_max:
            break
        function_details_list.append(str(count).rjust(column_widths[0]) + ' ' +
                '<a href="' + metric_link + '-' + urllib.parse.quote_plus(function_name) + '">' + function_name + '</a>\n')

    html += ''.join(function_details_list)
    html += '</pre>\n'
    html += '</body></html>\n'

    return html


# Lists all checkLibrary* messages regarding the given function name
def check_library_function_name(result_path: str, function_name: str, query_params: dict, nonfunc_id: str='') -> str:
    pkgs = '' if query_params.get('pkgs') == '1' else None
    function_name = urllib.parse.unquote_plus(function_name)
    if nonfunc_id:
        id = '[' + nonfunc_id
        marker = HEAD_MARKER
    else:
        if function_name.endswith('()'):
            id = '[checkLibrary'
        else:
            id = '[checkLibraryCheckType]'
        marker = INFO_MARKER
    output_lines_list = []
    for filename in glob.glob(result_path + '/*'):
        if not os.path.isfile(filename) or filename.endswith('.diff'):
            continue
        in_results = False
        package_url = None
        cppcheck_options = None
        for line in open(filename):
            if line.startswith('cppcheck: '):
                if OLD_VERSION not in line:
                    # Package results seem to be too old, skip
                    break
                # Current package, parse on
                continue
            if line.startswith('ftp://'):
                package_url = line
                continue
            if line.startswith('cppcheck-options:'):
                cppcheck_options = line
                continue
            if not in_results:
                if line.startswith(marker):
                    in_results = True
                continue
            if line.startswith('diff:'):
                break
            if id not in line:
                continue
            if not (' ' + function_name + ' ') in line:
                continue
            if pkgs is not None and package_url is not None:
                pkgs += f'{package_url.strip()}\n'
                break
            if package_url:
                output_lines_list.append(package_url)
                package_url = None
            if cppcheck_options:
                output_lines_list.append(cppcheck_options)
                cppcheck_options = None
            output_lines_list.append(line)

    if pkgs is not None:
        return pkgs
    return ''.join(output_lines_list)


def clientsReport(results_path: str):
    html = '<!DOCTYPE html>\n'
    html += '<html><head><title>Clients report</title></head><body>\n'
    html += '<h1>Clients report</h1>\n'
    current_year = datetime.date.today().year
    # TODO: use full profiles?
    # TODO: add jobs
    platforms = {}
    py_versions = {}
    client_versions = {}
    compilers = {}
    for filename in sorted(glob.glob(os.path.expanduser(results_path + '/*'))):
        if not os.path.isfile(filename) or filename.endswith('.diff'):
            continue
        with open(filename) as file_:
            datestr = None
            platform = None
            py_version = None
            client_version = None
            compiler = None
            for line in file_:
                line = line.strip()
                if line.startswith('cppcheck: '):
                    if OLD_VERSION not in line:
                        # Package results seem to be too old, skip
                        break
                    if not datestr:
                        break

                    dt = dateTimeFromStr(datestr)

                    if platform and not platform in platforms or dt < dateTimeFromStr(platforms[platform]):
                        platforms[platform] = datestr
                    if py_version and not py_version in py_versions or dt < dateTimeFromStr(py_versions[py_version]):
                        py_versions[py_version] = datestr
                    if client_version and not client_version in client_versions or dt < dateTimeFromStr(client_versions[client_version]):
                        client_versions[client_version] = datestr
                    if compiler and not compiler in compilers or dt < dateTimeFromStr(compilers[compiler]):
                        compilers[compiler] = datestr
                    break #  stop processing

                if datestr is None and line.startswith(str(current_year) + '-') or line.startswith(str(current_year - 1) + '-'):
                    datestr = line
                elif line.startswith('platform:'):
                    platform = line.split(' ', 1)[1]
                elif line.startswith('python:'):
                    py_version = line.split(' ',1 )[1]
                elif line.startswith('client-version:'):
                    client_version = line.split(' ', 1)[1]
                elif line.startswith('compiler:'):
                    compiler = line.split(' ', 1)[1]

    html += '<pre>\n'
    html += 'Client versions:\n'
    for clv in client_versions:
        html += clv + ' - ' + client_versions[clv] + '\n'
    html += '\n'
    html += 'Python versions:\n'
    for pyv in py_versions:
        html += pyv + ' - ' + py_versions[pyv] + '\n'
    html += '\n'
    html += 'Platforms:\n'
    for pl in platforms:
        html += pl + ' - ' + platforms[pl] + '\n'
    html += '\n'
    html += 'Compilers:\n'
    for cmp in compilers:
        html += cmp + ' - ' + compilers[cmp] + '\n'
    html += '</pre>\n'
    html += '</body></html>\n'
    return html


def sendAll(connection: socket.socket, text: str) -> None:
    data = text.encode('utf-8', 'ignore')
    while data:
        num = connection.send(data)
        if num < len(data):
            data = data[num:]
        else:
            data = None


def httpGetResponse(connection: socket.socket, data: str, contentType: str) -> None:
    resp = 'HTTP/1.1 200 OK\r\n'
    resp += 'Connection: close\r\n'
    resp += 'Content-length: ' + str(len(data)) + '\r\n'
    resp += 'Content-type: ' + contentType + '\r\n\r\n'
    resp += data
    sendAll(connection, resp)


class HttpClientThread(Thread):
    def __init__(self, connection: socket.socket, cmd: str, resultPath: str, latestResults: list) -> None:
        Thread.__init__(self)
        self.connection = connection
        self.cmd = cmd
        self.resultPath = resultPath
        self.infoPath = os.path.join(self.resultPath, 'info_output')
        self.latestResults = latestResults

    # TODO: use a proper parser
    @staticmethod
    def parse_req(cmd):
        req_parts = cmd.split(' ')
        if len(req_parts) != 3 or req_parts[0] != 'GET' or not req_parts[2].startswith('HTTP'):
            return None, None
        url_obj = urlparse(req_parts[1])
        return url_obj.path, dict(urllib.parse.parse_qsl(url_obj.query))

    def run(self):
        try:
            cmd = self.cmd
            url, queryParams = self.parse_req(cmd)
            if url is None:
                print_ts(f'invalid request: {cmd}')
                self.connection.close()
                return
            t_start = time.perf_counter()
            if url == '/':
                html = overviewReport()
                httpGetResponse(self.connection, html, 'text/html')
            elif url == '/latest.html':
                html = latestReport(self.latestResults)
                httpGetResponse(self.connection, html, 'text/html')
            elif url == '/crash.html':
                text, mime = crashReport(self.resultPath, queryParams)
                httpGetResponse(self.connection, text, mime)
            elif url == '/timeout.html':
                html = timeoutReport(self.resultPath)
                httpGetResponse(self.connection, html, 'text/html')
            elif url == '/stale.html':
                html = staleReport(self.resultPath, queryParams)
                httpGetResponse(self.connection, html, 'text/html')
            elif url == '/diff.html':
                html = diffReport(self.resultPath)
                httpGetResponse(self.connection, html, 'text/html')
            elif url.startswith('/difftoday-'):
                messageId = url[len('/difftoday-'):]
                text = diffMessageIdTodayReport(self.resultPath, messageId)
                httpGetResponse(self.connection, text, 'text/plain')
            elif url.startswith('/diff-'):
                messageId = url[len('/diff-'):]
                text = diffMessageIdReport(self.resultPath, messageId)
                httpGetResponse(self.connection, text, 'text/plain')
            elif url == '/head.html':
                html = headReport(self.resultPath)
                httpGetResponse(self.connection, html, 'text/html')
            elif url == '/headinfo.html':
                html = infoReport(self.infoPath)
                httpGetResponse(self.connection, html, 'text/html')
            elif url.startswith('/headtoday-'):
                messageId = url[len('/headtoday-'):]
                text = headMessageIdTodayReport(self.resultPath, messageId)
                httpGetResponse(self.connection, text, 'text/plain')
            elif url.startswith('/headinfotoday-'):
                messageId = url[len('/headinfotoday-'):]
                text = infoMessageIdTodayReport(self.infoPath, messageId)
                httpGetResponse(self.connection, text, 'text/plain')
            elif url.startswith('/head-'):
                messageId = url[len('/head-'):]
                text = headMessageIdReport(self.resultPath, messageId, queryParams)
                httpGetResponse(self.connection, text, 'text/plain')
            elif url.startswith('/headinfo-'):
                messageId = url[len('/headinfo-'):]
                text = infoMessageIdReport(self.infoPath, messageId, queryParams)
                httpGetResponse(self.connection, text, 'text/plain')
            elif url == '/time_lt.html':
                text, mime = timeReport(self.resultPath, False, queryParams)
                httpGetResponse(self.connection, text, mime)
            elif url == '/time_gt.html':
                text, mime = timeReport(self.resultPath, True, queryParams)
                httpGetResponse(self.connection, text, mime)
            elif url == '/time_slow.html':
                text = timeReportSlow(self.resultPath)
                httpGetResponse(self.connection, text, 'text/html')
            elif url == '/check_library_function_report.html':
                text = check_library_report(self.infoPath, message_id='checkLibraryFunction')
                httpGetResponse(self.connection, text, 'text/html')
            elif url == '/check_library_noreturn_report.html':
                text = check_library_report(self.infoPath, message_id='checkLibraryNoReturn')
                httpGetResponse(self.connection, text, 'text/html')
            elif url == '/check_library_use_ignore_report.html':
                text = check_library_report(self.infoPath, message_id='checkLibraryUseIgnore')
                httpGetResponse(self.connection, text, 'text/html')
            elif url == '/check_library_check_type_report.html':
                text = check_library_report(self.infoPath, message_id='checkLibraryCheckType')
                httpGetResponse(self.connection, text, 'text/html')
            elif url.startswith('/check_library-'):
                function_name = url[len('/check_library-'):]
                text = check_library_function_name(self.infoPath, function_name, queryParams)
                httpGetResponse(self.connection, text, 'text/plain')
            elif url == '/value_flow_bailout_incomplete_var.html':
                text = check_library_report(self.resultPath, message_id='valueFlowBailoutIncompleteVar')
                httpGetResponse(self.connection, text, 'text/html')
            elif url == '/unknown_macro.html':
                text = check_library_report(self.resultPath, message_id='unknownMacro')
                httpGetResponse(self.connection, text, 'text/html')
            elif url.startswith('/incomplete_var-'):
                var_name = url[len('/incomplete_var-'):]
                text = check_library_function_name(self.resultPath, var_name, queryParams, nonfunc_id='valueFlowBailoutIncompleteVar')
                httpGetResponse(self.connection, text, 'text/plain')
            elif url.startswith('/unknown_macro-'):
                var_name = url[len('/unknown_macro-'):]
                text = check_library_function_name(self.resultPath, var_name, queryParams, nonfunc_id='unknownMacro')
                httpGetResponse(self.connection, text, 'text/plain')
            elif url.startswith('/clients.html'):
                text = clientsReport(self.resultPath)
                httpGetResponse(self.connection, text, 'text/html')
            else:
                filename = self.resultPath + url
                if not os.path.isfile(filename):
                    print_ts('HTTP/1.1 404 Not Found')
                    self.connection.send(b'HTTP/1.1 404 Not Found\r\n\r\n')
                else:
                    with open(filename) as f:
                        data = f.read()
                    httpGetResponse(self.connection, data, 'text/plain')
            print_ts(f'{url} finished in {(time.perf_counter() - t_start)}s')
        except:
            tb = "".join(traceback.format_exception(sys.exc_info()[0], sys.exc_info()[1], sys.exc_info()[2]))
            print_ts(tb)
            httpGetResponse(self.connection, tb, 'text/plain')
        finally:
            time.sleep(1)
            self.connection.close()


def read_data(connection, cmd, pos_nl, max_data_size, check_done, cmd_name, timeout=10):
    data = cmd[pos_nl+1:]
    t = 0.0
    try:
        while (len(data) < max_data_size) and (not check_done or not data.endswith('\nDONE')) and (timeout > 0 and t < timeout):
            bytes_received = connection.recv(1024)
            if bytes_received:
                try:
                    text_received = bytes_received.decode('ascii', 'ignore')
                except UnicodeDecodeError as e:
                    print_ts(f'Error: Decoding failed ({cmd_name}): {e}')
                    data = None
                    break
                t = 0.0
                data += text_received
            elif not check_done:
                break
            else:
                time.sleep(0.2)
                t += 0.2
    except OSError as e:
        print_ts(f'Socket error occurred ({cmd_name}): {e}')
        data = None

    connection.close()

    if (timeout > 0) and (t >= timeout):
        print_ts(f'Timeout occurred ({cmd_name}).')
        data = None

    if data and (len(data) >= max_data_size):
        print_ts(f'Maximum allowed data ({max_data_size} bytes) exceeded ({cmd_name}).')

    elif data and check_done and not data.endswith('\nDONE'):
        print_ts(f'Incomplete data received ({cmd_name}).')
        data = None

    return data

def server(server_address_port: int, packages: list, packageIndex: int, resultPath: str) -> None:
    socket.setdefaulttimeout(30)
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server_address = ('', server_address_port)
    sock.bind(server_address)

    sock.listen(1)

    latestResults = []
    if os.path.isfile('latest.txt'):
        with open('latest.txt') as f:
            latestResults = f.read().strip().split(' ')

    print_ts('version ' + SERVER_VERSION)
    print_ts('listening on port ' + str(server_address_port))

    while True:
        # wait for a connection
        print_ts('waiting for a connection')
        connection, _ = sock.accept()
        try:
            bytes_received = connection.recv(128)
            cmd = bytes_received.decode('utf-8', 'ignore')
        except OSError as e:
            print_ts('Error: Recv error: ' + str(e))
            connection.close()
            continue
        except UnicodeDecodeError as e:
            print_ts('Error: Decoding failed: ' + str(e))
            connection.close()
            continue
        pos_nl = cmd.find('\n')
        if pos_nl < 1:
            print_ts(f"No newline found in data: '{cmd}'")
            connection.close()
            continue
        firstLine = cmd[:pos_nl]
        if re.match('[a-zA-Z0-9./ ]+', firstLine) is None:
            print_ts(f'Unsupported characters found in command: {firstLine}')
            connection.close()
            continue
        if cmd.startswith('GET /'):
            cmd = cmd[:cmd.find('\r\n')]
            print_ts(cmd)
            newThread = HttpClientThread(connection, cmd, resultPath, latestResults)
            newThread.start()
            continue
        if cmd == 'GetCppcheckVersions\n':
            reply = 'head ' + OLD_VERSION
            print_ts('GetCppcheckVersions: ' + reply)
            connection.send(reply.encode('utf-8', 'ignore'))
            connection.close()
            continue
        if cmd == 'get\n':
            while True:
                pkg = packages[packageIndex]
                packageIndex += 1
                if packageIndex >= len(packages):
                    packageIndex = 0
                if pkg is not None:
                    break

            with open('package-index.txt', 'w') as f:
                f.write(str(packageIndex) + '\n')

            print_ts('get:' + pkg)
            connection.send(pkg.encode('utf-8', 'ignore'))
            connection.close()
            continue
        if cmd.startswith('write\nftp://') or cmd.startswith('write\nhttp://'):
            t_start = time.perf_counter()
            data = read_data(connection, cmd, pos_nl, max_data_size=1024 * 1024, check_done=True, cmd_name='write')
            if data is None:
                continue
            truncated_data = len(data) >= 1024 * 1024

            pos = data.find('\n')
            if pos == -1:
                print_ts('No newline found in data. Ignoring result data.')
                continue
            if pos < 10:
                print_ts('Data is less than 10 characters. Ignoring result data.')
                continue
            url = data[:pos]
            print_ts('write:' + url)

            # save data
            res = re.match(r'ftp://.*pool/main/[^/]+/([^/]+)/[^/]*tar.(gz|bz2|xz)', url)
            if res is None:
                res = re.match(r'https?://cppcheck\.sf\.net/([a-z]+).tgz', url)
            if res is None:
                print_ts('res is None. Ignoring result data.')
                continue
            if url not in packages:
                print_ts('Url is not in packages. Ignoring result data.')
                continue
            # Verify that head was compared to correct OLD_VERSION
            versions_found = False
            old_version_wrong = False
            for line in data.split('\n', 20):
                if line.startswith('cppcheck: '):
                    versions_found = True
                    if OLD_VERSION not in line.split():
                        print_ts('Compared to wrong old version. Should be ' + OLD_VERSION + '. Versions compared: ' +
                              line + '. Ignoring result data.')
                        old_version_wrong = True
                    break
            if not versions_found:
                print_ts('Cppcheck versions missing in result data. Ignoring result data.')
                continue
            if old_version_wrong:
                print_ts('Unexpected old version. Ignoring result data.')
                continue
            filename = os.path.join(resultPath, res.group(1))
            if truncated_data:
                print_ts('Data is too large. Removing result.')
                if os.path.exists(filename):
                    os.remove(filename)
                continue
            with open(filename, 'w') as f:
                f.write(strDateTime() + '\n' + data)
            # track latest added results..
            if len(latestResults) >= 20:
                latestResults = latestResults[1:]
            latestResults.append(filename)
            with open('latest.txt', 'w') as f:
                f.write(' '.join(latestResults))
            # generate package.diff..
            generate_package_diff_statistics(filename)
            print_ts(f'write finished for {res.group(1)} ({len(data)} bytes / {(time.perf_counter() - t_start)}s)')
            continue
        if cmd.startswith('write_info\nftp://') or cmd.startswith('write_info\nhttp://'):
            t_start = time.perf_counter()
            data = read_data(connection, cmd, pos_nl, max_data_size=7 * 1024 * 1024, check_done=True, cmd_name='write_info')
            if data is None:
                continue
            truncated_data = len(data) >= 7 * 1024 * 1024

            pos = data.find('\n')
            if pos == -1:
                print_ts('No newline found in data. Ignoring information data.')
                continue
            if pos < 10:
                print_ts('Data is less than 10 characters. Ignoring information data.')
                continue
            url = data[:pos]
            print_ts('write_info:' + url)

            # save data
            res = re.match(r'ftp://.*pool/main/[^/]+/([^/]+)/[^/]*tar.(gz|bz2|xz)', url)
            if res is None:
                res = re.match(r'https://cppcheck\.sf\.net/([a-z]+).tgz', url)
            if res is None:
                print_ts('res is None. Ignoring information data.')
                continue
            if url not in packages:
                print_ts('Url is not in packages. Ignoring information data.')
                continue
            info_path = resultPath + '/' + 'info_output'
            if not os.path.exists(info_path):
                os.mkdir(info_path)
            filename = info_path + '/' + res.group(1)
            if truncated_data:
                print_ts('Data is too large. Removing result.')
                if os.path.exists(filename):
                    os.remove(filename)
                continue
            with open(filename, 'w') as f:
                f.write(strDateTime() + '\n' + data)
            print_ts(f'write_info finished for {res.group(1)} ({len(data)} bytes / {(time.perf_counter() - t_start)}s)')
            continue
        if cmd == 'getPackagesCount\n':
            packages_count = str(len(packages))
            connection.send(packages_count.encode('utf-8', 'ignore'))
            print_ts('getPackagesCount: ' + packages_count)
            connection.close()
            continue
        if cmd.startswith('getPackageIdx'):
            request_idx = abs(int(cmd[len('getPackageIdx:'):]))
            if request_idx < len(packages):
                pkg = packages[request_idx]
                connection.send(pkg.encode('utf-8', 'ignore'))
                print_ts('getPackageIdx: ' + pkg)
            else:
                print_ts(f'getPackageIdx: index {request_idx} is out of range')
            connection.close()
            continue
        if cmd.startswith('write_nodata\nftp://'):
            data = read_data(connection, cmd, pos_nl, max_data_size=8 * 1024, check_done=False, cmd_name='write_nodata')
            if data is None:
                continue

            pos = data.find('\n')
            if pos == -1:
                print_ts('No newline found in data. Ignoring no-data data.')
                continue
            if pos < 10:
                print_ts(f'Data is less than 10 characters ({pos}). Ignoring no-data data.')
                continue
            url = data[:pos]

            startIdx = packageIndex
            currentIdx = packageIndex
            while True:
                if packages[currentIdx] == url:
                    packages[currentIdx] = None
                    print_ts('write_nodata:' + url)

                    with open('packages_nodata.txt', 'a') as f:
                        f.write(url + '\n')
                    break
                if currentIdx == 0:
                    currentIdx = len(packages) - 1
                else:
                    currentIdx -= 1
                if currentIdx == startIdx:
                    print_ts('write_nodata:' + url + ' - package not found')
                    break
            continue

        if pos_nl < 0:
            print_ts('invalid command: "' + firstLine + '"')
        else:
            lines = cmd.split('\n')
            s = '\\n'.join(lines[:2])
            if len(lines) > 2:
                s += '...'
            print_ts('invalid command: "' + s + '"')
        connection.close()
        continue


if __name__ == "__main__":
    workPath = '/var/daca@home'
    if not os.path.isdir(workPath):
        workPath = os.path.expanduser('~/daca@home')
    os.chdir(workPath)
    print_ts('work path: ' + workPath)
    resultPath = workPath + '/donated-results'
    if not os.path.isdir(resultPath):
        print_ts(f"fatal: result path '{resultPath}' is missing")
        sys.exit(1)

    with open('packages.txt') as f:
        packages = [val.strip() for val in f.readlines()]

    print_ts(f'packages: {len(packages)}')

    if os.path.isfile('packages_nodata.txt'):
        with open('packages_nodata.txt') as f:
            packages_nodata = [val.strip() for val in f.readlines()]
            packages_nodata.sort()

        print_ts(f'packages_nodata: {len(packages_nodata)}')

        print_ts('removing packages with no files to process')
        packages_nodata_clean = []
        for pkg_n in packages_nodata:
            if pkg_n in packages:
                packages.remove(pkg_n)
                packages_nodata_clean.append(pkg_n)

        packages_nodata_diff = len(packages_nodata) - len(packages_nodata_clean)
        if packages_nodata_diff:
            with open('packages_nodata.txt', 'w') as f:
                for pkg in packages_nodata_clean:
                    f.write(pkg + '\n')

            print_ts(f'removed {packages_nodata_diff} packages from packages_nodata.txt')

        print_ts(f'packages: {len(packages)}')

    if len(packages) == 0:
        print_ts('fatal: there are no packages')
        sys.exit(1)

    packageIndex = 0
    if os.path.isfile('package-index.txt'):
        with open('package-index.txt') as f:
            packageIndex = int(f.read())
        if packageIndex < 0 or packageIndex >= len(packages):
            packageIndex = 0

    server_address_port = 8000
    if '--test' in sys.argv[1:]:
        server_address_port = 8001

    try:
        server(server_address_port, packages, packageIndex, resultPath)
    except socket.timeout:
        print_ts('Timeout!')
