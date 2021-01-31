# Copyright 2021 by Tim Seemann
# Released under the GNU General Public License.

import argparse
import json
import time
import datetime
import sys
import os
from CorDeployTool import CorDeployTool

def cli_to_args():
    """
    converts the command line interface to a series of args
    """
    cli = argparse.ArgumentParser(description="")
    cli.add_argument('-json_file',
                     type=str, required=True,
                     help='Location where deploy settings json is.')
    return cli.parse_args()


def seconds_to_time_str(secs):
    delta = datetime.timedelta(seconds=secs)
    return str(delta)


def run_build_from_json(json):
    """
    runs clean, then qmake, then make for most builds.
    """
    clean_build = json['clean_build']
    run_qmake = json['run_qmake']
    run_make = json['run_make']
    os.system(f"{clean_build}")
    os.system(f"{run_qmake}")
    os.system(f"{run_make}")


def run_build_android(json):
    """
    calls a standard build, then calls install and deploy for android.
    """
    # check for environment variables needed for android builds.
    sdk_root = os.getenv('ANDROID_SDK_ROOT', '')
    if len(sdk_root) == 0:
        sys.exit('Please define your ANDROID_SDK_ROOT.')
    ndk_root = os.getenv('ANDROID_NDK_ROOT', '')
    if len(ndk_root) == 0:
        sys.exit('Please define your ANDROID_NDK_ROOT.')
    # call a standard build
    run_build_from_json(json)
    # call install and deploy for android
    install_android = json['install_android']
    deploy_android_aab = json['deploy_android_aab']
    os.system(f"{install_android}")
    os.system(f"{deploy_android_aab}")


def main():
    args = cli_to_args()
    deploy_tool = CorDeployTool()
    overall_run_time = time.time()

    # read the json file
    input_file = open(args.json_file)

    # load the json
    deploy_json = json.load(input_file)

    """
    Run android
    """
    should_deploy_android = (len(deploy_json['android_deploy']) != 0)
    deploy_android_time = time.time()
    if should_deploy_android:
        android_json = deploy_json['android_deploy']
        run_build_android(android_json)
        deploy_tool.run_android(android_json['qt_build_dir'])
        deploy_android_time = time.time() - deploy_android_time

    """
    Run linux
    """
    should_deploy_linux = (len(deploy_json['linux_deploy']) != 0)
    deploy_linux_time = time.time()
    if should_deploy_android:
        linux_json = deploy_json['linux_deploy']
        run_build_from_json(linux_json)
        deploy_tool.run_linux(linux_json['qt_build_dir'])
        deploy_linux_time = time.time() - deploy_linux_time

    """
    Run Docs
    """
    should_deploy_docs = (deploy_json['docs_deploy'] == True)
    deploy_docs_time = time.time()
    if should_deploy_docs:
        deploy_tool.run_docs()
        deploy_docs_time = time.time() - deploy_docs_time

    """
    Print Results
    """
    print(f"-----------------")
    if should_deploy_android:
        print(f"Deploying android took {seconds_to_time_str(deploy_android_time)}.")
    if should_deploy_linux:
        print(f"Deploying linux took {seconds_to_time_str(deploy_linux_time)}.")
    if should_deploy_docs:
        print(f"Deploying docs took {seconds_to_time_str(deploy_docs_time)}.")

    overall_run_time =  time.time() - overall_run_time
    print(f"Overall runtime: {seconds_to_time_str(overall_run_time)}.")
    print(f"-----------------")


if __name__ == "__main__":
    main()
