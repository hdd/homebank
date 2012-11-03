
bin_PROGRAMS = homebank

homebank_SOURCES = \
gtkchart.c \
gtkdateentry.c \
amiga.c \
data_access.c \
def_account.c \
def_archive.c \
def_budget.c \
def_category.c \
def_filter.c \
def_lists.c \
def_operation.c \
def_payee.c \
def_pref.c \
def_wallet.c \
dsp_account.c \
dsp_wallet.c \
homebank.c \
list_account.c \
list_operation.c \
misc.c \
rep_budget.c \
rep_car.c \
rep_over.c \
rep_stats.c \
widgets.c \
wiz_import.c \
xml.c

homebank_LDADD = $(DEPS_LIBS)

AM_CPPFLAGS = $(DEPS_CFLAGS) \
	      -DLOCALE_DIR=\""$(datadir)/locale"\" \
	      -DPIXMAPS_DIR=\""$(datadir)/homebank/images"\" \
	      -DHELP_DIR=\""$(datadir)/homebank/help"\"
