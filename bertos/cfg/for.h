#ifndef MACROS_FOR_H
#define MACROS_FOR_H

#define IDENTITY(x) x

#define FOR(body, ...) \
		PP_CAT(FOR_, COUNT_PARMS(__VA_ARGS__)) (body, __VA_ARGS__)

#define FOR_1(body, x) IDENTITY(body x)

#define FOR_2(body, x, ...) \
		IDENTITY(body x) \
		FOR_1(body, __VA_ARGS__)

#define FOR_3(body, x, ...) \
		IDENTITY(body x) \
		FOR_2(body, __VA_ARGS__)

#define FOR_4(body, x, ...) \
		IDENTITY(body x) \
		FOR_3(body, __VA_ARGS__)

#define FOR_5(body, x, ...) \
		IDENTITY(body x) \
		FOR_4(body, __VA_ARGS__)

#define FOR_6(body, x, ...) \
		IDENTITY(body x) \
		FOR_5(body, __VA_ARGS__)

#define FOR_7(body, x, ...) \
		IDENTITY(body x) \
		FOR_6(body, __VA_ARGS__)

#define FOR_8(body, x, ...) \
		IDENTITY(body x) \
		FOR_7(body, __VA_ARGS__)

#define FOR_9(body, x, ...) \
		IDENTITY(body x) \
		FOR_8(body, __VA_ARGS__)

#define FOR_10(body, x, ...) \
		IDENTITY(body x) \
		FOR_9(body, __VA_ARGS__)

#define FOR_11(body, x, ...) \
		IDENTITY(body x) \
		FOR_10(body, __VA_ARGS__)

#define FOR_12(body, x, ...) \
		IDENTITY(body x) \
		FOR_11(body, __VA_ARGS__)

#define FOR_13(body, x, ...) \
		IDENTITY(body x) \
		FOR_12(body, __VA_ARGS__)

#define FOR_14(body, x, ...) \
		IDENTITY(body x) \
		FOR_13(body, __VA_ARGS__)

#define FOR_15(body, x, ...) \
		IDENTITY(body x) \
		FOR_14(body, __VA_ARGS__)

#define FOR_16(body, x, ...) \
		IDENTITY(body x) \
		FOR_15(body, __VA_ARGS__)

#define FOR_17(body, x, ...) \
		IDENTITY(body x) \
		FOR_16(body, __VA_ARGS__)

#define FOR_18(body, x, ...) \
		IDENTITY(body x) \
		FOR_17(body, __VA_ARGS__)

#define FOR_19(body, x, ...) \
		IDENTITY(body x) \
		FOR_18(body, __VA_ARGS__)

#define FOR_20(body, x, ...) \
		IDENTITY(body x) \
		FOR_19(body, __VA_ARGS__)

#define FOR_21(body, x, ...) \
		IDENTITY(body x) \
		FOR_20(body, __VA_ARGS__)

#define FOR_22(body, x, ...) \
		IDENTITY(body x) \
		FOR_21(body, __VA_ARGS__)

#define FOR_23(body, x, ...) \
		IDENTITY(body x) \
		FOR_22(body, __VA_ARGS__)

#define FOR_24(body, x, ...) \
		IDENTITY(body x) \
		FOR_23(body, __VA_ARGS__)

#define FOR_25(body, x, ...) \
		IDENTITY(body x) \
		FOR_24(body, __VA_ARGS__)

#define FOR_26(body, x, ...) \
		IDENTITY(body x) \
		FOR_25(body, __VA_ARGS__)

#define FOR_27(body, x, ...) \
		IDENTITY(body x) \
		FOR_26(body, __VA_ARGS__)

#define FOR_28(body, x, ...) \
		IDENTITY(body x) \
		FOR_27(body, __VA_ARGS__)

#define FOR_29(body, x, ...) \
		IDENTITY(body x) \
		FOR_28(body, __VA_ARGS__)

#define FOR_30(body, x, ...) \
		IDENTITY(body x) \
		FOR_29(body, __VA_ARGS__)

#define FOR_31(body, x, ...) \
		IDENTITY(body x) \
		FOR_30(body, __VA_ARGS__)

#define FOR_32(body, x, ...) \
		IDENTITY(body x) \
		FOR_31(body, __VA_ARGS__)

#endif
