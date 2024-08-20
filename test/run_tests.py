#!/usr/bin/env python3


import re
import sys
import textwrap
import subprocess
import os

from enum import Enum

class TimeOutError(Exception):
    pass

class ASanError(Exception):
    def __init__(self, message, errors):
        super().__init__(message)
        self.errors = errors

def split_test_name(test_name):
    cat, ex, case = test_name.split('.', 2)
    return cat, ex, case

class RES_SUCCESS:
    def __bool__(self):
        return True

class RES_FAILURE:
    def __init__(self, msg):
        self.msg = msg

    def __bool__(self):
        return False


class TestingUtils:
    SUCCESS = RES_SUCCESS
    FAILURE = RES_FAILURE

    def __init__(self, base_path, timeout_secs, verbose=False):
        self.timeout_secs = timeout_secs
        self.verbose = verbose
        self.base_path = base_path

    def run(self, executable, args, timeout_secs=-1, input=None):
        cmd = [executable] + args

        if self.verbose:
            print("  running command: {}".format(" ".join(cmd)))

        popen_args = {
                "stdout": subprocess.PIPE,
                "stderr": subprocess.PIPE,
            }

        env = dict()

        if timeout_secs <= 0:
            timeout_secs = self.timeout_secs

        try:
            cproc = subprocess.run(cmd, **popen_args, timeout=timeout_secs, encoding="utf-8", env=env, input=input)
        except subprocess.TimeoutExpired:
            raise TimeOutError()

        rc = cproc.returncode
        outs = cproc.stdout
        errs = cproc.stderr


        if "AddressSanitizer" in errs:
            msg = ""
            if len(outs) > 0:
                msg += "\nstdout:\n" + textwrap.indent(outs, "  ")
            if len(errs) > 0:
                msg += "\nstderr:\n" + textwrap.indent(errs, "  ")
            raise ASanError(message="AddressSanitizerError", errors=msg)

        return rc, outs, errs

    def check(self, run_res, fail_msg, exp_rc=0):
        rc, outs, errs = run_res

        if rc != exp_rc:
            msg = fail_msg
            if (len(outs) > 0):
                msg += "\nstdout:\n" + textwrap.indent(outs, "  ")
            if (len(errs) > 0):
                msg += "\nstderr:\n" + textwrap.indent(errs, "  ")
            return self.FAILURE(msg)

        return self.SUCCESS()

    def join_base(self, path):
        return os.path.join(self.base_path, path)

    def split_test_name(self, test_name):
        return split_test_name(test_name)

class Journaler:
    GREEN = '\033[92m'
    RED = '\033[91m'
    ENDC = '\033[0m'

    def __init__(self):
        self.curr_test = None
        self.num_fails = 0
        self.num_total = 0


    def start_test(self, test_name):
        assert self.curr_test is None
        self.curr_test = test_name
        print("Running test {}:".format(test_name))

    def success(self):
        assert self.curr_test is not None
        print("  {}PASS{}".format(self.GREEN, self.ENDC))
        self.curr_test = None
        self.num_total += 1

    def failure(self, msg):
        assert self.curr_test is not None
        while msg.endswith("\n"):
            msg = msg[:-1]
        self.num_fails += 1
        self.num_total += 1
        res_str = "  {}FAIL{}:".format(self.RED, self.ENDC)
        if "\n" in msg:
            first_line, remainder = msg.split("\n", 1)
            res_str += " {}\n".format(first_line)
            res_str += textwrap.indent(remainder, "    ")
        else:
            res_str += " {}".format(msg)
        print(res_str)
        self.curr_test = None

    def print_summary(self):
        if self.num_fails == 0:
            print("All {} tests successful!".format(self.num_total))
        else:
            print("{} out of {} tests failed!".format(self.num_fails, self.num_total))

def main():
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument('-t', '--testname', metavar='<testname>', default=None, help='only execute a test with this name')
    parser.add_argument('-f', '--filter', metavar='<regex>', default=None, help='only consider tests matching this regex')
    parser.add_argument('-l', '--list', action='store_true', help='only list tests, don\'t execute')
    parser.add_argument('-v', '--verbose', action='store_true', help='print executed commands for tests')

    args = parser.parse_args()

    try:
        from test_registry import all_tests
        from test_registry import timeout_secs
    except ImportError:
        print("No test_registry.py found!")
        sys.exit(1)

    test_dict = all_tests

    if args.filter is not None:
        r = re.compile(args.filter)
        test_dict = { k: v for k, v in test_dict.items() if r.fullmatch(k) }

    if args.list:
        for tn in test_dict.keys():
            print("{}".format(tn))
        sys.exit(0)

    if args.testname is None:
        test_names = test_dict.keys()
    else:
        test_names = [args.testname]

    journaler = Journaler()

    base_path = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

    testing_utils = TestingUtils(base_path=base_path, timeout_secs=timeout_secs, verbose=args.verbose)

    for test_name in test_names:
        journaler.start_test(test_name)

        test_fun = test_dict.get(test_name, None)
        if test_fun is None:
            journaler.failure("test not found")
            continue

        try:
            res = test_fun(testing_utils, test_name)
        except TimeOutError:
            journaler.failure("timeout error")
            continue
        except ASanError as asan_err:
            journaler.failure("ASAN error" + asan_err.errors)
            continue
        except Exception as E:
            journaler.failure("exotic error")
            print(E)
            continue
        except:
            journaler.failure("exotic error")
            continue

        assert isinstance(res, RES_SUCCESS) or isinstance(res, RES_FAILURE)

        if res:
            journaler.success()
        else:
            journaler.failure(res.msg)

    if len(test_names) > 1:
        journaler.print_summary()

    if journaler.num_fails > 0:
        sys.exit(1)

    sys.exit(0)


if (__name__ == '__main__'):
    main()

