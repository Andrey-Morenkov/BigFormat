#ifndef _ERRORS_H
	#define _ERRORS_H

	#define _QUOTE(x) # x
	#define QUOTE(x) _QUOTE(x)

	#ifdef _DEBUG

		#define INTERNAL_ERROR(name) \
			throw ("Internal Error in " QUOTE(__FILE__) "#" \
				QUOTE(__LINE__) ":" QUOTE(__FUNC__) ". " name)
		#define USER_ERROR(name) \
			throw ("Error detected in " QUOTE(__FILE__) "#" \
				QUOTE(__LINE__) ":" QUOTE(__FUNC__) ". " name)
		#define ASSERT(condition) \
			if (! (condition)) INTERNAL_ERROR("ASSERT failed for" QUOTE(condition))
		#define ASSERT_S(condition, text) \
			if (! (condition)) INTERNAL_ERROR(text)

	#else

		#define INTERNAL_ERROR(name) \
			throw ("Internal Error.")
		#define USER_ERROR(name) \
			throw ("Error. "name)
		#define ASSERT(condition)
		#define ASSERT_S(condition, text)

	#endif

	#define CHECK_USER(condition, text) \
		if (! (condition)) USER_ERROR(text)

	#define WARN_USER(condition, text) \
		if (! (condition)) printf("\nWARNING: %s\n", (text))

#endif