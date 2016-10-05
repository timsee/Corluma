#!/bin/bash

compile_doxygen()
{
    # remove previous versions
    rm -rf output
    mkdir output

    # run doxygen
    doxygen Doxyfile_${1}

    # if it contains a LaTeX version, compile as .pdf
    if [ -d output/latex ];
    then
        cd output/latex

        # build the LaTeX output
        make

        # copy the pdf
        cp -R refman.pdf ../${1}-API.pdf

        # remove LaTeX files
        cd ../..
        rm -rf output/latex
    fi
}

compile_doxygen "Corluma"



