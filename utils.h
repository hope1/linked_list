#ifndef UTILS_H_GUARD_
#define UTILS_H_GUARD_

#define Macro_util_swap(Macro_Arg_type, Macro_Arg_lhs, Macro_Arg_rhs) \
	do { \
		Macro_Arg_type _ = (Macro_Arg_rhs); \
		(Macro_Arg_rhs)  = (Macro_Arg_lhs); \
		(Macro_Arg_lhs)  = _; \
	} while(0)

#define Macro_declare_unused(Macro_Arg_var) \
	do ((void)(Macro_Arg_var)) ; while(0)

#endif
