# Copyright 2021 by Tim Seemann
# Released under the GNU General Public License.

import argparse
import fileinput
import os
from shutil import copyfile
from shutil import rmtree
from distutils.dir_util import copy_tree
from pathlib import Path

def cli_to_args():
    """
    converts the command line interface to a series of args
    """
    cli = argparse.ArgumentParser(description="")
    cli.add_argument('-env', type=str, required=True,
                     choices=["android", "linux", "docs"],
                     help='Environment to build.')
    cli.add_argument('-qt_build_dir',
                     type=str, required=False, default=argparse.SUPPRESS,
                     help='Qt build dir where the built application will be.')
    return cli.parse_args()


def replace_str_in_place(input_file, string_to_replace, new_string):
    with fileinput.FileInput(input_file, inplace=True) as file:
        for line in file:
            print(line.replace(string_to_replace, new_string), end='')


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


def deploy_android(qt_build_dir, version_str):
    aab_path = f"{qt_build_dir}/android-build/build/outputs/bundle/release/android-build-release.aab"
    print(f"aab_path: {aab_path}")
    Path(f"./output/").mkdir(parents=True, exist_ok=True)
    os.system(f"cp {aab_path} output/Corluma-{version_str}.aab")


def deploy_linux(qt_buid_dir, version_str):
    # convert version_str to linux standard
    version_list = version_str.split('.')
    if len(version_list) != 3:
        print("version_str invalid, cannot deploy linux")
        return
    linux_version_str = f"{version_list[0]}.{version_list[1]}-{version_list[2]}"

    # define parameters
    app_name = "corluma"
    deploy_dir = f"{app_name}_{linux_version_str}"
    deb_file = f"{deploy_dir}.deb"
    source_file = f"{qt_buid_dir}/{app_name}"
    dest_file = f"{deploy_dir}/usr/bin/{app_name}"

    # copy the linux .deb
    copy_tree('linux', deploy_dir)

    # find and replace the version string in deployed files
    replace_str_in_place(f"{deploy_dir}/DEBIAN/control",
                         f"Version: ~~VERSION_STRING~~",
                         f"Version: {linux_version_str}")
    replace_str_in_place(f"{deploy_dir}/usr/share/applications/corluma.desktop",
                         f"Version=~~VERSION_STRING~~",
                         f"Version={linux_version_str}")

    # Print file permission of the destination
    #perm = os.stat(source_file).st_mode
    #print(f"File permission before copy: {perm}")

    # copy the corluma binary to the bin directory
    os.system(f"cp -p {source_file} {dest_file}")

    # Print file permission of the destination
    #perm = os.stat(dest_file).st_mode
    #print(f"File permission after copy: {perm}")

    # run dpkg-build
    os.system(f"dpkg-deb --build {deploy_dir}")

    # make output directory if it doesn't exist
    Path(f"./output").mkdir(parents=True, exist_ok=True)
    # copy .deb to output directory
    os.system(f"cp -p {deb_file} output/{deb_file}")

    # remove working dir
    rmtree(deploy_dir)

    # remove original .deb
    os.remove(deb_file)


def deploy_docs(version_str):
    docs_workspace_path = "./workspace_docs"
    docs_output_path = f"{docs_workspace_path}/output"
    latex_path = f"{docs_output_path}/latex"
    doxyfile_path = f"{docs_workspace_path}/Doxyfile_Corluma"
    os.system(f"cp -rp ../docs/ {docs_workspace_path}")
    replace_str_in_place(f"{doxyfile_path}",
                         f"PROJECT_NUMBER = ~~VERSION_STRING~~",
                         f"PROJECT_NUMBER = v{version_str}")
    os.system(f"rm -rf {docs_output_path}")
    os.system(f"mkdir {docs_output_path}")
    # make output directory if it doesn't exist
    Path(f"./output/docs").mkdir(parents=True, exist_ok=True)
    # run doxygen
    os.system(f"doxygen {doxyfile_path}")
    # cleanup doxygen workspace
    os.system(f"rm -rf {docs_workspace_path}")


def main():
    args = cli_to_args()
    version_string = get_app_version()
    if version_string == "ERROR":
        print(f"Could not find the app version, check Corluma.pro.")
        return

    print(f"Building app version: {version_string}")
    if args.env == 'linux':
        if "qt_build_dir" not in args:
            print(f"Please provide a qt_build_dir.")
            return
        deploy_linux(args.qt_build_dir, version_string)
    elif args.env == 'android':
        if "qt_build_dir" not in args:
            print(f"Please provide a qt_build_dir.")
            return
        deploy_android(args.qt_build_dir, version_string)
    elif args.env == 'docs':
    	deploy_docs(version_string)
    #print(f"building env: {args.env} with output dir: {args.qt_build_dir}")


if __name__ == "__main__":
    main()
