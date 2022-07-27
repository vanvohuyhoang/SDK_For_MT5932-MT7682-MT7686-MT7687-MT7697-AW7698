#!/bin/bash

###############################################################################
#Variables
export PROJECT_LIST=$(find project | grep "GCC/Makefile$")
export FEATURE_LIST=$(find project | grep "GCC/feature.*.mk$")
export BOARD_LIST="project/*"
export OUT="$PWD/out"
export FLASHGENERATOR="tools/flashgen/flashgen.pl"
feature_mk=""
feature_mk_o=""
bl_feature_mk=""
bl_feature_mk_o=""
bl_feature_mk_o_flag=0

platform=$(uname)
if [[ "$platform" =~ "MINGW" ]]; then
    export EXTRA_VAR=-j
else
    export EXTRA_VAR=-j`cat /proc/cpuinfo |grep ^processor|wc -l`
fi
###############################################################################
#Functions
show_usage () {
    echo "==============================================================="
    echo "Build Project"
    echo "==============================================================="
    echo "Usage: $0 <board> <project> [bl|clean] <argument>"
    echo ""
    echo "Example:"
    echo "       $0 mt7687_hdk iot_sdk_demo"
    echo "       $0 mt7687_hdk iot_sdk_demo bl      (build with bootloader)"
    echo "       $0 clean                      (clean folder: out)"
    echo "       $0 mt7687_hdk clean           (clean folder: out/mt7687_hdk)"
    echo "       $0 mt7687_hdk iot_sdk_demo clean   (clean folder: out/mt7687_hdk/iot_sdk_demo)"
    echo ""
    echo "Argument:"
    echo "       -f=<feature makefile> or --feature=<feature makefile>"
    echo "           Replace feature.mk with other makefile. For example, "
    echo "           the feature_example.mk is under project folder, -f=feature_example.mk"
    echo "           will replace feature.mk with feature_example.mk."
    echo ""
    echo "       -o=<make option> or --option=<make option>"
    echo "           Assign additional make option. For example, "
    echo "           to compile module sequentially, use -o=-j1."
    echo "           to turn on specific feature in feature makefile, use -o=<feature_name>=y"
    echo "           to assign more than one options, use -o=<option_1> -o=<option_2>."
    echo ""
    echo "==============================================================="
    echo "List Available Example Projects"
    echo "==============================================================="
    echo "Usage: $0 list"
    echo ""
}

show_available_proj () {
    echo "==============================================================="
    echo "Available CM4 Build Projects:"
    #echo "total args passed to me $#"
    if [ $# -eq "2" ]; then
        board=$1
        project=$2
        echo "  board: $board"
        echo "  project: $project"
        echo "  "
        echo "  Example:      Using feature.mk as default for every project."
        echo "       $0 mt7687_hdk iot_sdk_demo"
        echo "==============================================================="
        features=$(echo $FEATURE_LIST | tr " " "\n" | grep "$board" |grep "$project")
        echo  "  "$board
        echo "  ""  "$project
        for f in $features
        do
            if [ "$(basename $f)" != "feature.mk" ]; then
                echo "  ""  ""    ""|-"$(basename $f)
            fi
        done
    else
        echo "  "
        echo "  Example:      Using feature.mk as default for every project."
        echo "  "
        echo "       $0 mt7687_hdk iot_sdk_demo"
        echo "==============================================================="
        for b in $BOARD_LIST
        do
            project_path=""
            feature_list=""
            p=$(echo $PROJECT_LIST | tr " " "\n" | grep "$b")
            if [ ! -z "$p" ]; then
                echo  "  "`basename $b`
            fi
            for q in $p
            do
                if [ -e "$q" ]; then
                    project_path=$(echo $q | sed 's/GCC\/Makefile//')
                    projectName=$(basename $project_path)
                    echo "  ""  "$projectName

                    features=$(echo $FEATURE_LIST | tr " " "\n" | grep "$b" |grep "$projectName")
                    for f in $features
                    do
                        if [ "$(basename $f)" != "feature.mk" ]; then
                            echo "  ""  ""    ""|-"$(basename $f)
                        fi
                    done
                fi
            done

        done
    fi
}

use_new_out_folder="false"
target_check () {
    for p in $PROJECT_LIST
    do
        q=$(echo $p | grep "project/$1/")
        if [ ! -z "$q" ]; then
            r=$(echo $q | sed 's/GCC\/Makefile//')
            s=`basename $r`
            if [ "$s" == "$2" ]; then
                echo "UE BUILD BOARD: $1"
                echo "UE BUILD PROJECT: $2"
                if [ $use_new_out_folder == "false" ]; then
                    OUT=$OUT/$1/$2
                fi
                BUILD=project/$1/$2
                export TARGET_PATH=$(dirname $q)
                return 0
            fi
        fi
    done
    return 1
}

# support MinGW
mingw_check () {
    echo "platform=$platform"
    if [[ "$platform" =~ "MINGW" ]]; then
        pwdpath=$(pwd)
        echo $pwdpath
        if [[ "$pwdpath" =~ "\[|\]| " ]]; then
            echo "Build.sh Exception: The codebase folder name should not have spacing, [ or ]."
            exit 1
        fi
    fi
}

clean_out () {
    rm -rf $1
    echo "rm -rf $1"
}
###############################################################################
#Begin here
if [ "$#" -eq "0" ]; then
    show_usage
    exit 1
fi

# parsing arguments
declare -a argv=($0)
ori_argv=$@
do_make_clean="none"
for i in $@
do
    case $i in
        -o=*|--option=*)
            opt=" ${i#*=}"
            echo "UE BUILD OPTION:$opt"
            echo "$opt" | grep -q -E " OUT="
            if [[ $? -eq 0 ]]; then
                OUT=`echo $opt | grep -o "OUT=[^ |^	]*" | cut -d '=' -f2 | tr -d ' '`
                if [ -z "$OUT" ]; then
                    echo "Error: -o=OUT= cannot be empty!"
                    show_usage
                    exit 1
                fi
                OUT=$PWD/$OUT
                use_new_out_folder="true"
                echo "output folder change to: $OUT"
            fi
            if [[ "$opt" =~ " FEATURE=" ]]; then
                feature_mk_o="${opt#*=}"
                shift
                continue
            fi
            if [[ "$opt" =~ " BL_FEATURE=" ]]; then
                bl_feature_mk_o="${opt#*=}"
                bl_feature_mk_o_flag=1
            fi
            extra_opt+=$opt
            shift
            ;;
        -blo=*|--bloption=*)
            opt=" ${i#*=}"
            echo "$opt" | grep -q -E " OUT="
            if [[ $? -eq 0 ]]; then
                echo "Error: Unsupported -o=OUT= in [-blo|-bloption]."
                exit 1
            fi
            bl_extra_opt+=$opt
            do_make_clean="true"
            shift
            ;;
        -f=*|--feature=*)
            feature_mk="${i#*=}"
            shift
            ;;
        list)
            if [ "${#argv[@]}" == "3" ]; then
                target_check ${argv[1]} ${argv[2]}
                if [ "$?" -eq "0" ]; then
                    show_available_proj ${argv[1]} ${argv[2]}
                    exit 0
                else
                    echo "Error: ${argv[1]} ${argv[2]} is not available board & project"
                    show_usage
                    exit 1
                fi
            elif [ "${#argv[@]}" == "1" ]; then
                show_available_proj
                exit 0
            else
                #echo "args : ${#argv[@]}"
                echo "Error: wrong usage!!"
                exit 1
            fi
            ;;
        -*)
            echo "Error: unknown parameter \"$i\""
            show_usage
            exit 1
            ;;
        *)
            argv+=($i)
            ;;
    esac
done

if [ ! -z $feature_mk_o ]; then
    if [ ! -z $feature_mk ]; then
        echo "Override -f=$feature_mk by -o=FEATURE=$feature_mk_o"
        echo "FEATURE=$feature_mk_o"
    fi
    feature_mk=$feature_mk_o
fi

export PROJ_NAME=${argv[2]}
###############################################################################
if [ "${argv[3]}" == "bl" ]; then
    if [ "${#argv[@]}" != "4" ]; then
        show_usage
        exit 1
    fi
    target_check ${argv[1]} ${argv[2]}
    if [ "$?" -ne "0" ]; then
        echo "Error: ${argv[1]} ${argv[2]} is not available board & project"
        show_usage
        exit 1
    fi

    mingw_check
    where_to_find_feature_mk=$TARGET_PATH
    if [ ! -z $feature_mk ]; then
        if [ ! -e "$TARGET_PATH/$feature_mk" ]; then
            echo "Error: cannot find $feature_mk under $TARGET_PATH."
            exit 1
        fi
        EXTRA_VAR+=" FEATURE=$feature_mk"
    else
        where_to_find_feature_mk=`grep "^TARGET_PATH\ *[?:]\{0,1\}=\ *" $TARGET_PATH/Makefile | cut -d '=' -f2 | tr -d ' ' | tail -1`
        if [ -z $where_to_find_feature_mk ]; then
            where_to_find_feature_mk=$TARGET_PATH
        fi
        feature_mk=`grep "^FEATURE\ *[?:]\{0,1\}=\ *" $TARGET_PATH/Makefile | cut -d '=' -f2 | tr -d ' ' | tail -1`
    fi
    echo "FEATURE = $feature_mk"

    if [ -e "$OUT/obj/$TARGET_PATH/tmp.mk" ]; then
        diff -q $where_to_find_feature_mk/$feature_mk $OUT/obj/$TARGET_PATH/tmp.mk
        if [ $? -ne 0 ]; then
            do_make_clean="true"
        fi
    fi
    if [ -e "$OUT/obj/$TARGET_PATH/extra_opts.lis" ]; then
        echo $extra_opt | grep -Eo  "[_[:alnum:]]+=[[:graph:]]*" | sort | uniq > $OUT/obj/$TARGET_PATH/extra_opts.current
        diff -q $OUT/obj/$TARGET_PATH/extra_opts.current $OUT/obj/$TARGET_PATH/extra_opts.lis
        if [ $? -ne 0 ]; then
            do_make_clean="true"
        else
            if [ $do_make_clean != "true" ]; then
                do_make_clean="false"
                rm -f $OUT/obj/$TARGET_PATH/extra_opts.current
            fi
        fi
    fi
    if [ $do_make_clean == "true" ]; then
        clean_out $OUT
    fi
    mkdir -p $OUT/obj/$TARGET_PATH
    echo $extra_opt | grep -Eo  "[_[:alnum:]]+=[[:graph:]]*" | sort | uniq > $OUT/obj/$TARGET_PATH/extra_opts.lis
    cp $where_to_find_feature_mk/$feature_mk $OUT/obj/$TARGET_PATH/tmp.mk

    CM4_TARGET_PATH_BAK=$TARGET_PATH
    TARGET_PATH="project/${argv[1]}/apps/bootloader/GCC"

    # Check bootloader feature makefile
    if [ "$bl_feature_mk_o_flag" -eq "0" ]; then
        bl_feature_mk=`grep "^BL_FEATURE\ *[?:]\{0,1\}=\ *" $CM4_TARGET_PATH_BAK/$feature_mk | cut -d '=' -f2 | tr -d ' ' | tail -1`
    else
        bl_feature_mk=$bl_feature_mk_o
    fi
    if [ ! -z $bl_feature_mk ]; then
        if [ ! -e "$TARGET_PATH/$bl_feature_mk" ]; then
            echo "Error: cannot find $bl_feature_mk under $TARGET_PATH."
            exit 1
        fi
        bl_extra_opt+=" FEATURE=$bl_feature_mk"
    fi
    echo "BL_FEATURE = $bl_feature_mk"

    mkdir -p $OUT/log
    echo "$0 $ori_argv" > $OUT/log/build_time.log
    echo "Start Build: "`date` >> $OUT/log/build_time.log
    echo "Build bootloader..."
    # Check if the source dir is existed
    if [ ! -d "project/${argv[1]}/apps/bootloader" ]; then
        echo "Error: no bootloader source in project/${argv[1]}/apps/bootloader"
        exit 1
    fi

    mkdir -p $OUT
    echo "make -C $TARGET_PATH BUILD_DIR=$OUT/obj/bootloader OUTPATH=$OUT BL_MAIN_PROJECT=${argv[2]} $bl_extra_opt"
    make -C $TARGET_PATH BUILD_DIR=$OUT/obj/bootloader OUTPATH=$OUT BL_MAIN_PROJECT=${argv[2]} $bl_extra_opt 2>> $OUT/err.log
    BUILD_RESULT=$?
    mkdir -p $OUT/lib
    mv -f $OUT/*.a $OUT/lib/ 2> /dev/null
    mkdir -p $OUT/log
    mv -f $OUT/*.log $OUT/log/ 2> /dev/null
    if [ $BUILD_RESULT -ne 0 ]; then
        echo "Error: bootloader build failed!!"
        echo "BOOTLOADER BUILD : FAIL" >> $OUT/log/build_time.log
        exit 2;
    else
        echo "BOOTLOADER BUILD : PASS" >> $OUT/log/build_time.log
    fi
    echo "Build bootloader...Done"

    # build cm4 firmware
    echo "Build CM4 Firmware..."
    TARGET_PATH=$CM4_TARGET_PATH_BAK
    mkdir -p $OUT/autogen
    EXTRA_VAR+="$extra_opt"
    echo "make -C $TARGET_PATH BUILD_DIR=$OUT/obj OUTPATH=$OUT $EXTRA_VAR"
    make -C $TARGET_PATH BUILD_DIR=$OUT/obj OUTPATH=$OUT $EXTRA_VAR 2>> $OUT/err.log
    BUILD_RESULT=$?
    mkdir -p $OUT/lib
    mv -f $OUT/*.a $OUT/lib/ 2> /dev/null
    mkdir -p $OUT/log
    mv -f $OUT/*.log $OUT/log/ 2> /dev/null
    echo "Build CM4 Firmware...Done"
    echo "End Build: "`date` >> $OUT/log/build_time.log
    cat $OUT/log/build.log | grep "MODULE BUILD" >> $OUT/log/build_time.log
    if [ "$BUILD_RESULT" -eq "0" ]; then
        echo "TOTAL BUILD: PASS" >> $OUT/log/build_time.log
    else
        echo "TOTAL BUILD: FAIL" >> $OUT/log/build_time.log
    fi
    echo "=============================================================="
    cat $OUT/log/build_time.log
    exit $BUILD_RESULT
elif [ "${argv[3]}" == "clean" ]; then
    if [ "${#argv[@]}" != "4" ]; then
        show_usage
        exit 1
    fi
    if [ "$use_new_out_folder" == "true" ]; then
        rm -rf $OUT
    else
        rm -rf $OUT/${argv[1]}/${argv[2]}
    fi
elif [ "${argv[2]}" == "clean" ]; then
    if [ "${#argv[@]}" != "3" ]; then
        show_usage
        exit 1
    fi
    if [ "$use_new_out_folder" == "true" ]; then
        rm -rf $OUT
    else
        rm -rf $OUT/${argv[1]}
    fi
elif [ "${argv[1]}" == "clean" ]; then
    if [ "${#argv[@]}" != "2" ]; then
        show_usage
        exit 1
    fi
    rm -rf $OUT
else
    if [ "${#argv[@]}" != "3" ]; then
        show_usage
        exit 1
    fi
    target_check ${argv[1]} ${argv[2]}
    if [ "$?" -ne "0" ]; then
        echo "Error: ${argv[1]} ${argv[2]} is not available board & project or module"
        show_usage
        exit 1
    fi

    mingw_check
    where_to_find_feature_mk=$TARGET_PATH
    if [ ! -z $feature_mk ]; then
        if [ ! -e "$TARGET_PATH/$feature_mk" ]; then
            echo "Error: cannot find $feature_mk under $TARGET_PATH."
            exit 1
        fi
        EXTRA_VAR+=" FEATURE=$feature_mk"
    else
        where_to_find_feature_mk=`grep "^TARGET_PATH\ *[?:]\{0,1\}=\ *" $TARGET_PATH/Makefile | cut -d '=' -f2 | tr -d ' ' | tail -1`
        if [ -z $where_to_find_feature_mk ]; then
            where_to_find_feature_mk=$TARGET_PATH
        fi
        feature_mk=`grep "^FEATURE\ *[?:]\{0,1\}=\ *" $TARGET_PATH/Makefile | cut -d '=' -f2 | tr -d ' ' | tail -1`
    fi
    echo "FEATURE = $feature_mk"

    if [ -e "$OUT/obj/$TARGET_PATH/tmp.mk" ]; then
        diff -q $where_to_find_feature_mk/$feature_mk $OUT/obj/$TARGET_PATH/tmp.mk
        if [ $? -ne 0 ]; then
            do_make_clean="true"
        fi
    fi
    if [ -e "$OUT/obj/$TARGET_PATH/extra_opts.lis" ]; then
        echo $extra_opt | grep -Eo  "[_[:alnum:]]+=[[:graph:]]*" | sort | uniq > $OUT/obj/$TARGET_PATH/extra_opts.current
        diff -q $OUT/obj/$TARGET_PATH/extra_opts.current $OUT/obj/$TARGET_PATH/extra_opts.lis
        if [ $? -ne 0 ]; then
            do_make_clean="true"
        else
            if [ $do_make_clean != "true" ]; then
                do_make_clean="false"
                rm -f $OUT/obj/$TARGET_PATH/extra_opts.current
            fi
        fi
    fi
    if [ $do_make_clean == "true" ]; then
        clean_out $OUT
    fi
    mkdir -p $OUT/obj/$TARGET_PATH
    echo $extra_opt | grep -Eo  "[_[:alnum:]]+=[[:graph:]]*" | sort | uniq > $OUT/obj/$TARGET_PATH/extra_opts.lis
    cp $where_to_find_feature_mk/$feature_mk $OUT/obj/$TARGET_PATH/tmp.mk
    mkdir -p $OUT/autogen
    mkdir -p $OUT/log
    echo "$0 $ori_argv" > $OUT/log/build_time.log
    echo "Start Build: "`date` >> $OUT/log/build_time.log
    if [ ! -z $feature_mk ]; then
        EXTRA_VAR+=" FEATURE=$feature_mk"
    fi
    EXTRA_VAR+="$extra_opt"
    #echo "make -C $TARGET_PATH BUILD_DIR=$OUT/obj OUTPATH=$OUT $EXTRA_VAR"
    make -C $TARGET_PATH BUILD_DIR=$OUT/obj OUTPATH=$OUT $EXTRA_VAR 2>> $OUT/err.log
    BUILD_RESULT=$?
    mkdir -p $OUT/lib
    mv -f $OUT/*.a $OUT/lib/ 2> /dev/null
    mv -f $OUT/*.log $OUT/log/ 2> /dev/null
    echo "End Build: "`date` >> $OUT/log/build_time.log
    cat $OUT/log/build.log | grep "MODULE BUILD" >> $OUT/log/build_time.log
    if [ "$BUILD_RESULT" -eq "0" ]; then
        echo "TOTAL BUILD: PASS" >> $OUT/log/build_time.log
    else
        echo "TOTAL BUILD: FAIL" >> $OUT/log/build_time.log
    fi
    echo "=============================================================="
    cat $OUT/log/build_time.log
    exit $BUILD_RESULT
fi

