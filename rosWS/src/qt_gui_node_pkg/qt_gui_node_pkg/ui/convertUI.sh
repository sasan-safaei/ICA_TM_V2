#!/bin/bash
#uic W_main.ui -o ./ui/uiW_main.h
#uic W_testing.ui -o ./ui/uiW_testing.h
pyuic5 W_main.ui -o ./py/uiW_main.py
pyuic5 W_testing.ui -o ./py/uiW_testing.py
pyuic5 W_ica2308.ui -o ./py/uiW_ica2308.py
