# Copyright 2021 by Tim Seemann
# Released under the GNU General Public License.

import argparse
import fileinput
import os
from shutil import copyfile
from shutil import rmtree
from distutils.dir_util import copy_tree


def cli_to_args():
    """
    converts the command line interface to a series of args
    """
    cli = argparse.ArgumentParser(description="")
    cli.add_argument('-env', type=str, required=True,
                     choices=["linux"],
                     help='Environment to build (currently only linux).')
    cli.add_argument('-qt_build_dir', type=str, required=True,
                     help='Qt build dir where the built application will be.')
    return cli.parse_args()


def get_app_version():
    app_path = "../src/Corluma.pro"
    version_variable = "VERSION = "
    f = open(app_path)
    pro_file = f.read()
    for line in pro_file.split("\n"):
        if version_variable in line:
            line = line.strip()
            line = line.replace(version_variable, '')
            return line
    return "ERROR"


def deploy_linux(qt_buid_dir, version_str):
    app_name = "corluma"
    # convert version_str to linux standard
    version_list = version_str.split('.')
    if len(version_list) != 3:
        print("version_str invalid, cannot deploy linux")
        return
    linux_version_str = f"{version_list[0]}.{version_list[1]}-{version_list[2]}"

    deploy_dir = f"{app_name}_{linux_version_str}"
    # copy the linux .deb
    copy_tree('linux', deploy_dir)

    # find and replace the version string
    with fileinput.FileInput(f"{deploy_dir}/DEBIAN/control", inplace=True) as file:
        for line in file:
            print(line.replace("Version: ~~VERSION_STRING~~", f"Version: {linux_version_str}"), end='')

    # copy the corluma binary to the bin directory
    copyfile(f"{qt_buid_dir}/{app_name}", f"{deploy_dir}/usr/bin/{app_name}")

    # run dpkg-build
    os.system(f"dpkg-deb --build {deploy_dir}")

    # remove working dir
    rmtree(deploy_dir)


def main():
    args = cli_to_args()
    version_string = get_app_version()
    if version_string == "ERROR":
        print(f"Could not find the app version, check Corluma.pro.")

    print(f"version string: {version_string}")
    if args.env == 'linux':
        deploy_linux(args.qt_build_dir, version_string)
    print(f"building env: {args.env} with output dir: {args.qt_build_dir}")


if __name__ == "__main__":
    main()

