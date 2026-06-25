#!/usr/bin/env python3

# Donate CPU
#
# A script a user can run to donate CPU to cppcheck project
#
# Syntax: donate-cpu.py [-jN] [--package=url] [--stop-time=HH:MM] [--work-path=path] [--test] [--bandwidth-limit=limit]
#  -jN                  Use N threads in compilation/analysis. Default is 1.
#  --package=url        Check a specific package and then stop. Can be useful if you want to reproduce
#                       some warning/crash/exception/etc..
#  --stop-time=HH:MM    Stop analysis when time has passed. Default is that you must terminate the script.
#  --work-path=path     Work folder path. Default path is cppcheck-donate-cpu-workfolder in your home folder.
#  --test               Connect to a donate-cpu-server that is running locally on port 8001 for testing.
#  --bandwidth-limit=limit Limit download rate for packages. Format for limit is the same that wget uses.
#                       Examples: --bandwidth-limit=250k => max. 250 kilobytes per second
#                                 --bandwidth-limit=2m => max. 2 megabytes per second
#  --max-packages=N     Process N packages and then exit. A value of 0 means infinitely.
#  --no-upload          Do not upload anything. Defaults to False.
#  --packages           Process a list of given packages.
#  --version            Returns the version (of the underlying donate_cpu_lib.py).
#
# What this script does:
# 1. Check requirements
# 2. Pull & compile Cppcheck
# 3. Select a package
# 4. Download package
# 5. Analyze source code
# 6. Upload results
# 7. Repeat from step 2
#
# Quick start: just run this script without any arguments

import platform
import os
import sys
import re
import time
import subprocess
import donate_cpu_lib as lib

from packaging.version import Version

__my_script_name = os.path.splitext(os.path.basename(sys.argv[0]))[0]
work_path = os.path.expanduser(os.path.join('~', 'cppcheck-' + __my_script_name + '-workfolder'))
max_packages = None
package_urls = []
do_upload = True
bandwidth_limit = None
stop_time = None

for arg in sys.argv[1:]:
    # --stop-time=12:00 => run until ~12:00 and then stop
    if arg.startswith('--stop-time='):
        stop_time = arg[-5:]
        print('Stop time:' + stop_time)
    elif arg.startswith('-j'):
        if not re.match(r'-j\d+', arg):
            print('Argument "{}" is invalid.'.format(arg))
            print('"-j" must be followed by a positive number.')
            sys.exit(1)
        print('Jobs:' + arg[2:])
        lib.set_jobs(arg)
    elif arg.startswith('--package='):
        pkg = arg[arg.find('=')+1:]
        package_urls.append(pkg)
        print('Added Package:' + pkg)
    elif arg.startswith('--packages='):
        pkg_cnt = len(package_urls)
        with open(arg[arg.find('=')+1:], 'rt') as f:
            for package_url in f:
                package_url = package_url.strip()
                if not package_url:
                    continue
                package_urls.append(package_url)
        print('Added Packages:' + str(len(package_urls) - pkg_cnt))
    elif arg.startswith('--work-path='):
        work_path = os.path.abspath(arg[arg.find('=')+1:])
        print('work_path:' + work_path)
        if not os.path.exists(work_path):
            print('work path does not exist!')
            sys.exit(1)
    elif arg == '--test':
        lib.set_server_address(('localhost', 8001))
    elif arg.startswith('--bandwidth-limit='):
        bandwidth_limit = arg[arg.find('=')+1:]
    elif arg.startswith('--max-packages='):
        arg_value = arg[arg.find('=')+1:]
        try:
            max_packages = int(arg_value)
        except ValueError:
            max_packages = None
        if max_packages < 0:
            max_packages = None
        if max_packages is None:
            print('Error: Max. packages value "{}" is invalid. Must be a positive number or 0.'.format(arg_value))
            sys.exit(1)
        # 0 means infinitely, no counting needed.
        if max_packages == 0:
            max_packages = None
    elif arg.startswith('--no-upload'):
        do_upload = False
    elif arg == '--version':
        print(lib.get_client_version())
        sys.exit(0)
    elif arg == '--help':
        print('Donate CPU to Cppcheck project')
        print('')
        print('Syntax: donate-cpu.py [-jN] [--stop-time=HH:MM] [--work-path=path]')
        print('  -jN                  Use N threads in compilation/analysis. Default is 1.')
        print('  --package=url        Check a specific package and then stop. Can be useful if you want to reproduce')
        print('                       some warning/crash/exception/etc..')
        print('  --stop-time=HH:MM    Stop analysis when time has passed. Default is that you must terminate the script.')
        print('  --work-path=path     Work folder path. Default path is ' + work_path)
        print('  --bandwidth-limit=limit Limit download rate for packages. Format for limit is the same that wget uses.')
        print('                       Examples: --bandwidth-limit=250k => max. 250 kilobytes per second')
        print('                                 --bandwidth-limit=2m => max. 2 megabytes per second')
        print('  --max-packages=N     Process N packages and then exit. A value of 0 means infinitely.')
        print('  --no-upload          Do not upload anything. Defaults to False.')
        print('  --packages           Process a list of given packages.')
        print('  --version            Returns the version (of the underlying donate_cpu_lib.py).')
        print('')
        print('Quick start: just run this script without any arguments')
        sys.exit(0)
    else:
        print('Unhandled argument: ' + arg)
        sys.exit(1)

if sys.version_info.major < 3 or (sys.version_info.major == 3 and sys.version_info.minor < 7):
    print("#" * 80)
    print("IMPORTANT")
    print("Please run the client with at least Python 3.7, thanks!")
    print("#" * 80)
    time.sleep(2)
    sys.exit(1)

print('Thank you!')
if not lib.check_requirements():
    sys.exit(1)
if bandwidth_limit and isinstance(bandwidth_limit, str):
    if subprocess.call(['wget', '--limit-rate=' + bandwidth_limit, '-q', '--spider', 'cppcheck1.osuosl.org']) == 2:
        print('Error: Bandwidth limit value "' + bandwidth_limit + '" is invalid.')
        sys.exit(1)
    else:
        print('Bandwidth-limit: ' + bandwidth_limit)
if package_urls:
    max_packages = len(package_urls)
if max_packages:
    print('Maximum number of packages to download and analyze: {}'.format(max_packages))
if not os.path.exists(work_path):
    os.mkdir(work_path)
repo_path = os.path.join(work_path, 'repo')
# This is a temporary migration step which should be removed in the future
migrate_repo_path = os.path.join(work_path, 'cppcheck')

packages_processed = 0

print('Get Cppcheck..')
try:
    lib.try_retry(lib.clone_cppcheck, fargs=(repo_path, migrate_repo_path))
except Exception as e:
    print('Error: Failed to clone Cppcheck ({}), retry later'.format(e))
    sys.exit(1)

while True:
    if max_packages:
        if packages_processed >= max_packages:
            print('Processed the specified number of {} package(s). Exiting now.'.format(max_packages))
            break
        print('Processing package {} of the specified {} package(s).'.format(packages_processed + 1, max_packages))
        packages_processed += 1
    if stop_time:
        print('stop_time:' + stop_time + '. Time:' + time.strftime('%H:%M') + '.')
        if stop_time < time.strftime('%H:%M'):
            print('Stopping. Thank you!')
            sys.exit(0)
    try:
        cppcheck_versions = lib.try_retry(lib.get_cppcheck_versions, max_tries=3, sleep_duration=30.0, sleep_factor=1.0)
    except Exception as e:
        print('Failed to get cppcheck versions from server ({}), retry later'.format(e))
        sys.exit(1)
    for ver in cppcheck_versions:
        if ver == 'head':
            ver = 'main'
        current_cppcheck_dir = os.path.join(work_path, 'tree-'+ver)
        if ver != 'main' and lib.has_binary(current_cppcheck_dir):
            print('No need to check Cppcheck-{} for changes - binary already exists'.format(ver))
            continue
        print('Checking Cppcheck-{} for changes..'.format(ver))
        try:
            has_changes = lib.try_retry(lib.checkout_cppcheck_version, fargs=(repo_path, ver, current_cppcheck_dir), max_tries=3, sleep_duration=30.0, sleep_factor=1.0)
        except KeyboardInterrupt as e:
            # Passthrough for user abort
            raise e
        except Exception as e:
            print('Failed to update Cppcheck-{} ({}), retry later'.format(ver, e))
            sys.exit(1)
        if ver == 'main':
            if (has_changes or not lib.has_binary(current_cppcheck_dir)) and not lib.compile_cppcheck(current_cppcheck_dir):
                print('Failed to compile Cppcheck-{}, retry later'.format(ver))
                sys.exit(1)
        else:
            if not lib.compile_version(current_cppcheck_dir):
                print('Failed to compile Cppcheck-{}, retry later'.format(ver))
                sys.exit(1)
    if package_urls:
        package = package_urls[packages_processed-1]
    else:
        try:
            package = lib.get_package()
        except Exception as e:
            print('Error: Failed to get package ({}), retry later'.format(e))
            sys.exit(1)
    tgz = lib.download_package(work_path, package, bandwidth_limit)
    if tgz is None:
        print("No package downloaded")
        continue
    skip_files = None
    if package.find('/qtcreator/') > 0:
        # macro_pounder_fn.c is a preprocessor torture test that takes time to finish
        skip_files = ('macro_pounder_fn.c',)
    source_path, source_found = lib.unpack_package(work_path, tgz, skip_files=skip_files)
    if not source_found:
        print("No files to process")
        if do_upload:
            lib.upload_nodata(package)
            print('Sleep 5 seconds..')
            time.sleep(5)
        continue
    crash = False
    timeout = False
    count = ''
    elapsed_time = ''
    results_to_diff = []
    cppcheck_options = ''
    head_info_msg = ''
    head_timing_info = ''
    old_timing_info = ''
    cppcheck_head_info = ''
    client_version_head = ''
    libraries = lib.library_includes.get_libraries(source_path)

    for ver in cppcheck_versions:
        tree_path = os.path.join(work_path, 'tree-'+ver)
        capture_callstack = False
        if ver == 'head':
            tree_path = os.path.join(work_path, 'tree-main')
            cppcheck_head_info = lib.get_cppcheck_info(tree_path)
            capture_callstack = True

            def get_client_version_head(path):
                cmd = 'python3' + ' ' + os.path.join(path, 'tools', 'donate-cpu.py') + ' ' + '--version'
                with subprocess.Popen(cmd.split(), stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, universal_newlines=True) as p:
                    try:
                        # TODO: handle p.returncode?
                        stdout, _ = p.communicate()
                    except:
                        return None
                return stdout.strip()

            client_version_head = get_client_version_head(tree_path)
        c, errout, info, t, cppcheck_options, timing_info = lib.scan_package(tree_path, source_path, libraries, capture_callstack)
        if c < 0:
            if c == -101 and 'error: could not find or open any of the paths given.' in errout:
                # No sourcefile found (for example only headers present)
                count += ' 0'
            elif c == lib.RETURN_CODE_TIMEOUT:
                # Timeout
                count += ' TO!'
                timeout = True
            else:
                crash = True
                count += ' Crash!'
        else:
            count += ' ' + str(c)
        elapsed_time += " {:.1f}".format(t)
        errout = errout.replace(work_path, '[...]')
        results_to_diff.append(errout)
        if ver == 'head':
            head_info_msg = info
            head_timing_info = timing_info
        else:
            old_timing_info = timing_info

    output = 'cppcheck-options: ' + cppcheck_options + '\n'
    output += 'platform: ' + platform.platform() + '\n'
    output += 'python: ' + platform.python_version() + '\n'
    output += 'client-version: ' + lib.get_client_version() + '\n'
    output += 'compiler: ' + lib.get_compiler_version() + '\n'
    output += 'cppcheck: ' + ' '.join(cppcheck_versions) + '\n'
    output += 'head-info: ' + cppcheck_head_info + '\n'
    output += 'count:' + count + '\n'
    output += 'elapsed-time:' + elapsed_time + '\n'
    output += 'head-timing-info:\n' + head_timing_info + '\n'
    output += 'old-timing-info:\n' + old_timing_info + '\n'
    info_output = output
    info_output += 'info messages:\n' + head_info_msg
    if 'head' in cppcheck_versions:
        output += 'head results:\n' + results_to_diff[cppcheck_versions.index('head')]
    if not crash and not timeout:
        output += 'diff:\n' + lib.diff_results(cppcheck_versions[0], results_to_diff[0], cppcheck_versions[1], results_to_diff[1]) + '\n'
    if package_urls:
        print('=========================================================')
        print(output)
        print('=========================================================')
        print(info_output)
        print('=========================================================')
    if do_upload:
        if lib.upload_results(package, output):
            lib.upload_info(package, info_output)
    if not max_packages or packages_processed < max_packages:
        print('Sleep 5 seconds..')
        if (client_version_head is not None) and (Version(client_version_head) > Version(lib.get_client_version())):
            print("ATTENTION: A newer client version ({}) is available - please update!".format(client_version_head))
        time.sleep(5)
