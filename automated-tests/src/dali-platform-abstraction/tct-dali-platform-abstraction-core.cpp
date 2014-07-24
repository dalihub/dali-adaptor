/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#include <stdio.h>
#include <string.h>
#include "tct-dali-platform-abstraction-core.h"

int main(int argc, const char *argv[])
{
    int result = -1;
    int i;

    if (argc != 2) {
        printf("Usage: %s <testcase name>\n", argv[0]);
        return 2;
    }

    for (i = 0; tc_array[i].name; i++) {
        if (!strcmp(argv[1], tc_array[i].name)) {
            if (tc_array[i].startup)
                tc_array[i].startup();

            result = tc_array[i].function();

            if (tc_array[i].cleanup)
                tc_array[i].cleanup();

            return result;
        }
    }

    printf("Unknown testcase name: \"%s\"\n", argv[1]);
    return 2;
}
