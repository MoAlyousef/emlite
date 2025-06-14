#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// these are freestanding headers
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#if __has_include(<stdlib.h>)
#include <stdlib.h>
#else
void *malloc(size_t);
void *realloc(void *, size_t);
void free(void *);
#endif

#if __has_include(<string.h>)
#include <string.h>
#else
size_t strlen(const char *);
#endif

#ifndef EMLITE_USED
#define EMLITE_USED __attribute__((used))
#endif

typedef uint32_t Handle;

typedef Handle (*Callback)(Handle);

// externs
Handle emlite_val_null(void);
Handle emlite_val_undefined(void);
Handle emlite_val_false(void);
Handle emlite_val_true(void);
Handle emlite_val_global_this();
Handle emlite_val_new_array(void);
Handle emlite_val_new_object(void);
char *emlite_val_typeof(Handle);
Handle emlite_val_construct_new(Handle, Handle argv);
Handle emlite_val_func_call(Handle func, Handle argv);
void emlite_val_push(Handle arr, Handle v);
Handle emlite_val_make_int(int t);
Handle emlite_val_make_double(double t);
Handle emlite_val_make_str(const char *, size_t);
int emlite_val_get_value_int(Handle);
double emlite_val_get_value_double(Handle);
char *emlite_val_get_value_string(Handle);
Handle emlite_val_get_elem(Handle, size_t);
bool emlite_val_is_string(Handle);
bool emlite_val_is_number(Handle);
bool emlite_val_not(Handle);
bool emlite_val_gt(Handle, Handle);
bool emlite_val_gte(Handle, Handle);
bool emlite_val_lt(Handle, Handle);
bool emlite_val_lte(Handle, Handle);
bool emlite_val_equals(Handle, Handle);
bool emlite_val_strictly_equals(Handle, Handle);
bool emlite_val_instanceof(Handle, Handle);
void emlite_val_delete(Handle);
void emlite_val_throw(Handle);

Handle emlite_val_obj_call(
    Handle obj, const char *name, size_t len, Handle argv
);
Handle emlite_val_obj_prop(Handle obj, const char *prop, size_t len);
void emlite_val_obj_set_prop(
    Handle obj, const char *prop, size_t len, Handle val
);
bool emlite_val_obj_has_prop(Handle, const char *prop, size_t len);
bool emlite_val_obj_has_own_prop(Handle, const char *prop, size_t len);
Handle emlite_val_make_callback(Handle id);
void *emlite_malloc(size_t);
void *emlite_realloc(void *, size_t);
void emlite_free(void *);
// end externs

typedef struct {
    Handle h;
} em_Val;

em_Val em_Val_from_int(int i);
em_Val em_Val_from_double(double i);
em_Val em_Val_from_string(const char *i);
em_Val em_Val_from_handle(uint32_t v);
em_Val em_Val_global(const char *name);
em_Val em_Val_global_this();
em_Val em_Val_null();
em_Val em_Val_undefined();
em_Val em_Val_object();
em_Val em_Val_array();
em_Val em_Val_make_js_function(Callback f);
void em_Val_delete(em_Val);
void em_Val_throw(em_Val);

Handle em_Val_as_handle(em_Val self);
em_Val em_Val_get(em_Val self, const char *prop);
void em_Val_set(em_Val self, const char *prop, em_Val val);
bool em_Val_has(em_Val self, const char *prop);
bool em_Val_has_own_property(em_Val self, const char *prop);
char *em_Val_typeof(em_Val self);
em_Val em_Val_at(em_Val self, size_t idx);
em_Val em_Val_await(em_Val self);
bool em_Val_is_number(em_Val self);
bool em_Val_is_string(em_Val self);
bool em_Val_instanceof(em_Val self, em_Val v);
bool em_Val_not(em_Val self);
bool em_Val_seq(em_Val self, em_Val other);
bool em_Val_eq(em_Val self, em_Val other);
bool em_Val_neq(em_Val self, em_Val other);
bool em_Val_gt(em_Val self, em_Val other);
bool em_Val_gte(em_Val self, em_Val other);
bool em_Val_lt(em_Val self, em_Val other);
bool em_Val_lte(em_Val self, em_Val other);

int em_Val_as_int(em_Val self);
bool em_Val_as_bool(em_Val self);
double em_Val_as_double(em_Val self);
char *em_Val_as_string(em_Val self);

em_Val em_Val_call(em_Val self, const char *method, int n, ...);
em_Val em_Val_new(em_Val self, int n, ...);
em_Val em_Val_invoke(em_Val self, int n, ...);

em_Val emlite_eval(const char *src);

em_Val emlite_eval_v(const char *src, ...);
#define EMLITE_EVAL(x, ...) emlite_eval_v(#x __VA_OPT__(, __VA_ARGS__))

#ifdef EMLITE_IMPL

// present in freestanding environments
#include <stdarg.h>

#if __has_include(<string.h>)
#include <string.h>
#else
size_t strlen(const char *s) {
    const char *p = s;
    while (*p)
        ++p;
    return (size_t)(p - s);
}
#endif

#if __has_include(<stdlib.h>)
#include <stdlib.h>
#else
void *malloc(size_t s) {
    return emlite_malloc(s);
}

void *realloc(void * p, size_t s) {
    return emlite_realloc(p, s);
}

void free(void *p) {
    emlite_free(p);
}
#endif

#if __has_include(<stdio.h>)
#include <stdio.h>
#else
static size_t emlite_utoa(
    char *buf, unsigned long long value, int base, int upper
) {
    static const char digits_low[] = "0123456789abcdef";
    static const char digits_up[]  = "0123456789ABCDEF";
    const char *digits             = upper ? digits_up : digits_low;

    size_t i = 0;
    if (value == 0) {
        buf[i++] = '0';
    } else {
        while (value) {
            buf[i++] = digits[value % base];
            value /= base;
        }

        for (size_t j = 0; j < i / 2; ++j) {
            char t         = buf[j];
            buf[j]         = buf[i - 1 - j];
            buf[i - 1 - j] = t;
        }
    }
    return i;
}

static inline __attribute__((__always_inline__)) int vsnprintf(
    char *out, size_t n, const char *fmt, va_list ap
) {
    size_t pos = 0;

    while (*fmt) {
        if (*fmt != '%') {
            if (pos + 1 < n)
                out[pos] = *fmt;
            ++pos;
            ++fmt;
            continue;
        }

        ++fmt;

        if (*fmt == '%') {
            if (pos + 1 < n)
                out[pos] = '%';
            ++pos;
            ++fmt;
            continue;
        }

        int longflag = 0;
        while (*fmt == 'l') {
            ++longflag;
            ++fmt;
        }

        char tmp[32];
        const char *chunk = tmp;
        size_t chunk_len  = 0;
        int negative      = 0;

        switch (*fmt) {
        case 's': {
            const char *s = va_arg(ap, const char *);
            if (!s)
                s = "(null)";
            chunk     = s;
            chunk_len = strlen(s);
            break;
        }
        case 'c': {
            tmp[0]    = (char)va_arg(ap, int);
            chunk     = tmp;
            chunk_len = 1;
            break;
        }
        case 'd':
        case 'i': {
            long long v = longflag ? va_arg(ap, long long) : va_arg(ap, int);
            if (v < 0) {
                negative = 1;
                v        = -v;
            }
            chunk_len = emlite_utoa(tmp, (unsigned long long)v, 10, 0);
            break;
        }
        case 'u': {
            unsigned long long v = longflag ? va_arg(ap, unsigned long long)
                                            : va_arg(ap, unsigned int);
            chunk_len            = emlite_utoa(tmp, v, 10, 0);
            break;
        }
        case 'x':
        case 'X': {
            unsigned long long v = longflag ? va_arg(ap, unsigned long long)
                                            : va_arg(ap, unsigned int);
            chunk_len            = emlite_utoa(tmp, v, 16, (*fmt == 'X'));
            break;
        }
        default:
            tmp[0]    = '%';
            tmp[1]    = *fmt;
            chunk     = tmp;
            chunk_len = 2;
            break;
        }

        if (negative) {
            if (pos + 1 < n)
                out[pos] = '-';
            ++pos;
        }

        for (size_t i = 0; i < chunk_len; ++i) {
            if (pos + 1 < n)
                out[pos] = chunk[i];
            ++pos;
        }

        ++fmt;
    }

    if (n)
        out[(pos < n) ? pos : (n - 1)] = '\0';

    return (int)pos;
}

static inline __attribute__((__always_inline__)) int snprintf(
    char *out, size_t n, const char *fmt, ...
) {
    va_list ap;
    va_start(ap, fmt);
    int r = vsnprintf(out, n, fmt, ap);
    va_end(ap);
    return r;
}
#endif

em_Val em_Val_from_int(int i) { return (em_Val){.h = emlite_val_make_int(i)}; }

em_Val em_Val_from_double(double i) {
    return (em_Val){.h = emlite_val_make_double(i)};
}
em_Val em_Val_from_string(const char *s) {
    return (em_Val){.h = emlite_val_make_str(s, strlen(s))};
}

em_Val em_Val_from_handle(uint32_t v) { return (em_Val){.h = v}; }

em_Val em_Val_global(const char *name) {
    Handle global = emlite_val_global_this();
    return em_Val_from_handle(emlite_val_obj_prop(global, name, strlen(name)));
}

em_Val em_Val_global_this() {
    return em_Val_from_handle(emlite_val_global_this());
}

em_Val em_Val_null() { return em_Val_from_handle(0); }

em_Val em_Val_undefined() { return em_Val_from_handle(1); }

em_Val em_Val_object() { return em_Val_from_handle(emlite_val_new_object()); }

em_Val em_Val_array() { return em_Val_from_handle(emlite_val_new_array()); }

em_Val em_Val_make_js_function(Callback f) {
    uint32_t fidx = (uint32_t)f;
    return em_Val_from_handle(emlite_val_make_callback(fidx));
}

void em_Val_delete(em_Val v) { emlite_val_delete(v.h); }

void em_Val_throw(em_Val v) { emlite_val_throw(v.h); }

Handle em_Val_as_handle(em_Val self) { return self.h; }

em_Val em_Val_get(em_Val self, const char *prop) {
    return em_Val_from_handle(emlite_val_obj_prop(self.h, prop, strlen(prop)));
}

void em_Val_set(em_Val self, const char *prop, em_Val val) {
    emlite_val_obj_set_prop(self.h, prop, strlen(prop), val.h);
}

bool em_Val_has(em_Val self, const char *prop) {
    return emlite_val_obj_has_prop(self.h, prop, strlen(prop));
}

bool em_Val_has_own_property(em_Val self, const char *prop) {
    return emlite_val_obj_has_own_prop(self.h, prop, strlen(prop));
}

char *em_Val_typeof(em_Val self) { return emlite_val_typeof(self.h); }

em_Val em_Val_at(em_Val self, size_t idx) {
    return em_Val_from_handle(emlite_val_get_elem(self.h, idx));
}

em_Val em_Val_await(em_Val self) {
    char buf[256];
    const char *code = "(async() => { let obj = ValMap.toValue(%d); let ret = "
                       "await obj; return ValMap.toHandle(ret); })()";
    (void)snprintf(buf, 256, code, self.h);
    return emlite_eval(buf);
}

bool em_Val_is_number(em_Val self) { return emlite_val_is_number(self.h); }

bool em_Val_is_string(em_Val self) { return emlite_val_is_string(self.h); }

bool em_Val_instanceof(em_Val self, em_Val v) {
    return emlite_val_instanceof(self.h, v.h);
}

bool em_Val_not(em_Val self) { return emlite_val_not(self.h); }

bool em_Val_seq(em_Val self, em_Val other) {
    return emlite_val_strictly_equals(self.h, other.h);
}

bool em_Val_eq(em_Val self, em_Val other) {
    return emlite_val_equals(self.h, other.h);
}

bool em_Val_neq(em_Val self, em_Val other) {
    return !emlite_val_strictly_equals(self.h, other.h);
}

bool em_Val_gt(em_Val self, em_Val other) {
    return emlite_val_gt(self.h, other.h);
}

bool em_Val_gte(em_Val self, em_Val other) {
    return emlite_val_gte(self.h, other.h);
}

bool em_Val_lt(em_Val self, em_Val other) {
    return emlite_val_lt(self.h, other.h);
}

bool em_Val_lte(em_Val self, em_Val other) {
    return emlite_val_lte(self.h, other.h);
}

int em_Val_as_int(em_Val self) { return emlite_val_get_value_int(self.h); }

bool em_Val_as_bool(em_Val self) { return self.h > 3; }

double em_Val_as_double(em_Val self) {
    return emlite_val_get_value_double(self.h);
}

char *em_Val_as_string(em_Val self) {
    return emlite_val_get_value_string(self.h);
}

em_Val em_Val_call(em_Val self, const char *method, int n, ...) {
    Handle arr = emlite_val_new_array();
    va_list args;
    va_start(args, n);
    for (int i = 0; i < n; i++) {
        em_Val c = va_arg(args, em_Val);
        emlite_val_push(arr, em_Val_as_handle(c));
    }
    va_end(args);
    return em_Val_from_handle(
        emlite_val_obj_call(self.h, method, strlen(method), arr)
    );
}

em_Val em_Val_new(em_Val self, int n, ...) {
    Handle arr = emlite_val_new_array();
    va_list args;
    va_start(args, n);
    for (int i = 0; i < n; i++) {
        em_Val c = va_arg(args, em_Val);
        emlite_val_push(arr, em_Val_as_handle(c));
    }
    va_end(args);
    return em_Val_from_handle(emlite_val_construct_new(self.h, arr));
}

em_Val em_Val_invoke(em_Val self, int n, ...) {
    Handle arr = emlite_val_new_array();
    va_list args;
    va_start(args, n);
    for (int i = 0; i < n; i++) {
        em_Val c = va_arg(args, em_Val);
        emlite_val_push(arr, em_Val_as_handle(c));
    }
    va_end(args);
    return em_Val_from_handle(emlite_val_func_call(self.h, arr));
}

em_Val emlite_eval(const char *src) {
    em_Val eval = em_Val_global("eval");
    return em_Val_invoke(eval, 1, em_Val_from_string(src));
}

em_Val emlite_eval_v(const char *src, ...) {
    va_list args;
    va_start(args, src);
    va_list args_len;
    va_copy(args_len, args);
    size_t len = vsnprintf(NULL, 0, src, args_len);
    va_end(args_len);
    char *ptr = (char *)malloc(len + 1);
    // check if ptr was allocated
    vsnprintf(ptr, len + 1, src, args);
    va_end(args);
    em_Val ret = emlite_eval(ptr);
    free(ptr);
    return ret;
}

#endif

#ifdef __cplusplus
}
#endif