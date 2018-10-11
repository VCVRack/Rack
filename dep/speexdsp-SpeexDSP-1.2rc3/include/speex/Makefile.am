# Disable automatic dependency tracking if using other tools than gcc and gmake
#AUTOMAKE_OPTIONS = no-dependencies

# Historically, these headers were part of the speex package, so in order to
# keep compatibility with existing software we need to keep installing them
# in the same place as they were found then.  If new functionality, with new
# headers, are added later, some thought should be given as to whether they
# should instead go somewhere other than $prefix/include/speex.

speexincludedir = $(includedir)/speex

nodist_speexinclude_HEADERS = speexdsp_config_types.h

speexinclude_HEADERS = speex_echo.h speex_jitter.h speex_preprocess.h speex_resampler.h \
	speexdsp_types.h

noinst_HEADERS=speex_buffer.h

