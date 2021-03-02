#!/usr/bin/env python3

# Copyright 2021 by Tim Seemann
# Released under the GNU General Public License.

import argparse
import json
import time
import datetime
import sys
import os
from pathlib import Path

def cli_to_args():
    """
    converts the command line interface to a series of args
    """
    cli = argparse.ArgumentParser(description="")
    cli.add_argument('-icon_file',
                     type=str, required=True,
                     help='Location where icon file is.')
    return cli.parse_args()


def seconds_to_time_str(secs):
    delta = datetime.timedelta(seconds=secs)
    return str(delta)


def generate_icon(input, output_dir, output_name, side):
    output_file = f"{output_dir}/{output_name}.png"
    convert_image = f"magick convert {input} -resize {side}x{side} {output_file}"
    print(convert_image)
    os.system(convert_image)
    return output_file


def make_icons(input):
    # generate all the iOS size variants
    # https://doc.qt.io/qt-5/ios-platform-notes.html
    ios_output_dir = f"./output/appIcon"
    Path(ios_output_dir).mkdir(parents=True, exist_ok=True)
    ios_output_files = [
        ("AppIcon29x29", 29),
        ("AppIcon29x29@2x", 58),
        ("AppIcon29x29@2x~ipad", 58),
        ("AppIcon29x29~ipad", 29),
        ("AppIcon40x40@2x", 80),
        ("AppIcon40x40@2x~ipad", 80),
        ("AppIcon40x40~ipad", 40),
        ("AppIcon50x50@2x~ipad", 100),
        ("AppIcon50x50~ipad", 50),
        ("AppIcon57x57", 57),
        ("AppIcon57x57@2x~ipad", 114),
        ("AppIcon60x60@2x", 120),
        ("AppIcon72x72@2x~ipad", 144),
        ("AppIcon72x72~ipad", 72),
        ("AppIcon76x76@2x~ipad", 152),
        ("AppIcon76x76~ipad", 76)
    ]
    for file, size in ios_output_files:
        generate_icon(input, ios_output_dir, file, size)

    # generate all the mac size variants
    # https://stackoverflow.com/questions/12306223/how-to-manually-create-icns-files-using-iconutil
    mac_output_dir = f"./output/icon.iconset"
    Path(mac_output_dir).mkdir(parents=True, exist_ok=True)
    mac_output_files = [
        ("icon_16x16", 16),
        ("icon_16x16@2x", 32),
        ("icon_32x32", 32),
        ("icon_32x32@2x", 64),
        ("icon_128x128", 128),
        ("icon_128x128@2x", 256),
        ("icon_256x256", 256),
        ("icon_256x256", 512),
        ("icon_512x512", 512),
        ("icon_512x512@2x", 1024)
    ]
    for file, size in mac_output_files:
        generate_icon(input, mac_output_dir, file, size)

    # create a icns file.
    make_icns = f"iconutil -c icns {mac_output_dir}"
    print(make_icns)
    os.system(make_icns)

    # cleanup
    os.system(f"rm -rf {mac_output_dir}")


    # https://superuser.com/questions/227736/how-do-i-convert-a-png-into-a-ico
    # create .ico files
    ico_output_dir = f"./output/tmp_ico"
    Path(ico_output_dir).mkdir(parents=True, exist_ok=True)
    ico_output_files = [
        ("24", 24),
        ("32", 32),
        ("40", 40),
        ("48", 48),
        ("64", 64),
        ("96", 96),
        ("128", 128),
        ("256", 256)
    ]
    ico_file_paths = []
    for file, size in mac_output_files:
        ico_file_paths.append(generate_icon(input, ico_output_dir, file, size))

    ico_script = f"convert {' '.join(ico_file_paths)} ./output/icon.ico"
    print(ico_script)
    os.system(ico_script)

    # clean up
    os.system(f"rm -rf {ico_output_dir}")

    # add an icon for docs
    docs_output_dir = f"./output/docs"
    Path(docs_output_dir).mkdir(parents=True, exist_ok=True)
    generate_icon(input, docs_output_dir, "icon", 50)


def main():
    args = cli_to_args()
    overall_run_time = time.time()

    make_icons(args.icon_file)

    overall_run_time =  time.time() - overall_run_time
    print(f"Overall runtime: {seconds_to_time_str(overall_run_time)}.")


if __name__ == "__main__":
    main()
