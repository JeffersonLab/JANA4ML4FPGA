#!/usr/bin/env python3
#
#
# nm -o install/bin/jana4ml4fpga | awk '{print $3" "$2}' | sort > jana4ml4fpga.nm.out
# nm -o ../JANA2/build/install/lib/libJANA.so | awk '{print $3" "$2}' | sort > libjana.so.nm.out

with open('jana4ml4fpga.nm.out') as f:
    lines1 = [line.rstrip() for line in f]
    with open('libjana.so.nm.out') as f:
        lines2 = [line.rstrip() for line in f]
        for line in lines1:
            if line in lines2:
                if line.endswith(' U'):
                    continue
                if 'asan' in line:
                    continue
                print(line)
