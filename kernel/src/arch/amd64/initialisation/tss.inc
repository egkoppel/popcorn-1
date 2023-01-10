%ifndef TSS_INC
%define TSS_INC

struc tss
    ._res0 resb 4
    .rsp0 resb 8
    .rsp1 resb 8
    .rsp2 resb 8
    ._res1 resb 8
    .ist1 resb 8
    .ist2 resb 8
    .ist3 resb 8
    .ist4 resb 8
    .ist5 resb 8
    .ist6 resb 8
    .ist7 resb 8
    ._res2 resb 8
    ._res3 resb 2
    .io_map_base resb 2
endstruc

%endif ; TSS_INC
