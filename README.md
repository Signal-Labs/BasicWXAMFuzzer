# BasicWXAMFuzzer

## Info
See: https://www.signal-labs.com/blog/fuzzing-wechats-wxam-parser

## Jackalope Command Line:
fuzzer.exe -in C:\tmp\wxam_inputs -out C:\tmp\jOut -t 30000 -nthreads 2 -delivery shmem -max_sample_size 1000000 -instrument_module WeChatWin.dll -instrument_module voipEngine.dll -persist -target_module WxamFuzzer.exe -target_method fuzz -nargs 0 -iterations 80000 -cmp_coverage -- WxamFuzzer.exe @@

## Notes
WxamFuzzer.exe needs to find the voipEngine.dll and WeChatWin.dll files, alongside their dependancies (so the DLLs need to be in the search path or folder), additionally
the target tries to find its own data folder (usually in C:\ProgramData\Tencent), so you'll need these too. The easiest way would be to simply install WeChat on your fuzzing machine before fuzzing, you can do this in a VM instead of your host.

Also note you'll have to update the pointer offsets in the harness to match your version of WeChatWin.dll (assuming the functions haven't undergone breaking changes between whatever version you're using, and the version I tested)
