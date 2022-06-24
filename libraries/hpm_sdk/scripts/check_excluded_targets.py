# Copyright 2022 hpmicro
# SPDX-License-Identifier: BSD-3-Clause

import sys
import yaml

EXCLUDED_TARGETS="excluded_targets"

def get_excluded_targets(input_yml):
    excluded = []
    with open(input_yml, "r") as stream:
        try:
            info = yaml.safe_load(stream)
            if EXCLUDED_TARGETS in info.keys():
                for t in info[EXCLUDED_TARGETS]:
                    excluded.append(t.strip().lower())
        except yaml.YAMLError as e:
            pass
    stream.close()
    if len(excluded):
        sys.stdout.write("%s" % (";".join(excluded)))

if __name__ == "__main__":
    get_excluded_targets(sys.argv[1])

