
common_defines = \
	-DSHARE_DIR=\""$(pkgdatadir)"\" \
	-DDATA_DIR=\""$(datadir)"\"


bin_PROGRAMS = homebank

DATA_ACCESS = \
	da_account.c  \
	da_account.h \
	da_assign.c \
	da_assign.h \
	da_encoding.c \
	da_encoding.h \
	da_payee.c  \
	da_payee.h \
	da_category.c \
	da_category.h \
	da_other.c \
	da_other.h \
	da_transaction.c \
	da_transaction.h \
	da_tag.c \
	da_tag.h

BUSINESS_CORE = \
	hb_account.c \
	hb_account.h \
	hb_category.c \
	hb_category.h \
	hb_payee.c \
	hb_payee.h \
	hb_transaction.c \
	hb_transaction.h

USER_INTERFACE = \
	ui_account.c \
	ui_account.h \
	ui_assign.c \
	ui_assign.h \
	ui_payee.c \
	ui_payee.h \
	ui_category.c \
	ui_category.h \
	ui-assist-start.c \
	ui-assist-start.h

homebank_SOURCES = \
	$(DATA_ACCESS) \
	$(BUSINESS_CORE) \
	$(USER_INTERFACE) \
	gtkchart.c \
	gtkdateentry.c \
	def_archive.c \
	def_budget.c \
	def_filter.c \
	def_operation.c \
	def_pref.c \
	def_wallet.c \
	dsp_account.c \
	dsp_wallet.c \
	homebank.c \
	imp_amiga.c \
	import.c \
	imp_qif.c \
	list_account.c \
	list_operation.c \
	list_upcoming.c \
	misc.c \
	preferences.c \
	rep_budget.c \
	rep_car.c \
	rep_over.c \
	rep_stats.c \
	rep_time.c \
	widgets.c \
	xml.c \
	def_archive.h \
	def_budget.h \
	def_filter.h \
	def_operation.h \
	def_pref.h \
	def_wallet.h \
	dsp_account.h \
	dsp_wallet.h \
	enums.h \
	gtkchart.h \
	gtkdateentry.h \
	homebank.h \
	imp_amiga.h \
	import.h \
	imp_qif.h \
	list_account.h \
	list_operation.h \
	list_upcoming.h \
	misc.h \
	preferences.h \
	rep_budget.h \
	rep_car.h \
	rep_over.h \
	rep_stats.h \
	rep_time.h \
	widgets.h \
	xml.h 

homebank_LDADD = $(DEPS_LIBS)

AM_CPPFLAGS = \
	$(DEPS_CFLAGS) \
	$(common_defines)

