ENTRY(_init)

SECTIONS
{
    .discard : {
        __noncacheable_start__ = .;
        __noncacheable_bss_start__ = .;
        __bss_start__ = .;
        __bss_end__ = .;
        __noncacheable_bss_end__ = .;
        _end = .;
        __noncacheable_init_start__ = .;
        __data_start__ = .;
        __data_end__ = .;
        __noncacheable_init_end__ = .;
        __noncacheable_end__ = .;
        __heap_start__ = .;
        __heap_end__ = .;
        __ramfunc_start__ = .;
        __ramfunc_end__ = .;
        __noncacheable_bss_start__ = .;
        __noncacheable_bss_end__ = .;
        __noncacheable_init_start__ = .;
        __noncacheable_init_end__ = .;
    }
    .text : {
        *(.text)
        *(.text*)
        *(.rodata)
        KEEP(*(.flash_algo.text*))
        KEEP(*(.flash_algo.data*))
        __etext = .;
    }
}
