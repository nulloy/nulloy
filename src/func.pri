defineReplace(fixSlashes) {
    win32:1 ~= s,/,\\,g
    return($$1)
}

defineTest(mkdir) {
	DIR = $$fixSlashes($$1)
	!exists($$DIR):system(mkdir $$DIR)
}

