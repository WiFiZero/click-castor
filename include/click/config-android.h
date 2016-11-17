/* include/click/config-android.h.  Generated from config-android.h.in by configure.  */
/* Process this file with configure to produce config-android.h. -*- mode: c -*- */
#ifndef CLICK_CONFIG_ANDROID_H
#define CLICK_CONFIG_ANDROID_H

/* Include userlevel configuration. */
#define CLICK_USERLEVEL 1
#include <click/config-userlevel.h>

/* Define if you have the <execinfo.h> header file. */
#undef HAVE_EXECINFO_H

/* Define if you have the <ifaddrs.h> header file. */
#undef HAVE_IFADDRS_H

#undef HAVE_NET_BPF_H

/* Define if you have -lpcap and pcap.h. */
#undef HAVE_PCAP

#undef HAVE_ALLOW_KQUEUE

/* Define if the C++ compiler understands static_assert. */
#undef HAVE_CXX_STATIC_ASSERT

#ifdef __cplusplus

/* Define macros that surround Click declarations. */
#ifndef CLICK_DECLS
# define CLICK_DECLS		/* */
# define CLICK_ENDDECLS		/* */
# define CLICK_USING_DECLS	/* */
# define CLICK_NAME(name)	::name
#endif

#endif /* __cplusplus */

#endif /* CLICK_CONFIG_ANDROID_H */
