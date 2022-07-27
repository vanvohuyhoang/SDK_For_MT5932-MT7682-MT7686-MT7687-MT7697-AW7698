ROOT            = ..\..\..\..\..
NAME            = dev_remove
SRCDIR          = ..
IS_32           = TRUE
WANT_C1132      = TRUE
WANT_WDMDDK     = TRUE
IS_DDK          = TRUE

L32EXE          = $(NAME).exe
L32RES          = .\$(NAME).res
L32LIBSNODEP    = kernel32.lib user32.lib gdi32.lib comctl32.lib libc.lib cfgmgr32.lib
TARGETS         = $(L32EXE)
DEPENDNAME      = $(SRCDIR)\depend.mk
RCFLAGS         = -I$(ROOT)\DEV\INC

# Enable read-only string pooling to coalesce all the redundant strings
#
CFLAGS          = -GF

L32OBJS         =   call_tskill.obj

!INCLUDE $(ROOT)\DEV\MASTER.MK

