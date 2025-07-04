#if __has_include(<new>)
#else
void *operator new(size_t size) { return malloc(size); }

void *operator new[](size_t size) { return malloc(size); }

void operator delete(void *val) noexcept { free(val); }

void operator delete[](void *val) noexcept { free(val); }

void *operator new(size_t, void *place) noexcept {
    return place;
}
#endif
namespace emlite {
Val::Val() noexcept : v_(0) {}

Val::Val(const Val &other) noexcept : v_(other.v_) {
    if (v_)
        emlite_val_inc_ref(v_);
}
Val &Val::operator=(const Val &other) noexcept {
    if (this == &other)
        return *this;
    if (v_)
        emlite_val_dec_ref(v_);
    v_ = other.v_;
    if (v_)
        emlite_val_inc_ref(v_);
    return *this;
}

Val &Val::operator=(Val &&other) noexcept {
    if (this != &other) {
        if (v_)
            emlite_val_dec_ref(v_);
        v_       = other.v_;
        other.v_ = 0;
    }
    return *this;
}

Val::Val(Val &&other) noexcept : v_(other.v_) {
    other.v_ = 0;
}

Val::~Val() {
    if (v_)
        emlite_val_dec_ref(v_);
}

Val Val::clone() const noexcept {
    return Val(*this);
}

Val Val::take_ownership(Handle h) noexcept {
    Val v;
    v.v_ = h;
    return v;
}

Val Val::global(const char *v) noexcept {
    return Val::take_ownership(emlite_val_global_this())
        .get(v);
}

Val Val::global() noexcept {
    return Val::take_ownership(emlite_val_global_this());
}

Val Val::null() noexcept { return Val::take_ownership(0); }

Val Val::undefined() noexcept { return Val::take_ownership(1); }

Val Val::object() noexcept {
    return Val::take_ownership(emlite_val_new_object());
}

Val Val::array() noexcept {
    return Val::take_ownership(emlite_val_new_array());
}

Val Val::dup(Handle h) noexcept {
    emlite_val_inc_ref(h);
    return Val::take_ownership(h);
}

Handle Val::release(Val &&v) noexcept {
    auto temp = v.v_;
    v.v_ = 0;
    return temp;
}

void Val::delete_(Val &&v) noexcept { emlite_val_dec_ref(v.v_); }

void Val::throw_(const Val &v) { return emlite_val_throw(v.v_); }

Handle Val::as_handle() const noexcept { return v_; }

Uniq<char[]> Val::type_of() const noexcept {
    return Uniq<char[]>(emlite_val_typeof(v_));
}

bool Val::has_own_property(const char *prop) const noexcept {
    return emlite_val_obj_has_own_prop(
        v_, prop, strlen(prop)
    );
}

Val Val::make_fn(Callback f) noexcept {
    uint32_t fidx =
        static_cast<uint32_t>(reinterpret_cast<uintptr_t>(f)
        );
    return Val::take_ownership(emlite_val_make_callback(fidx
    ));
}

// clang-format off
Val Val::await() const {
    return emlite_eval_cpp(
        "(async() => { let obj = EMLITE_VALMAP.toValue(%d); let ret = await obj; "
        "return EMLITE_VALMAP.toHandle(ret); })()",
        v_
    );
}
// clang-format on

bool Val::is_number() const noexcept {
    return emlite_val_is_number(v_);
}

bool Val::is_string() const noexcept {
    return emlite_val_is_string(v_);
}

bool Val:: instanceof (const Val &v) const noexcept {
    return emlite_val_instanceof(v_, v.v_);
}

bool Val::operator!() const { return emlite_val_not(v_); }

bool Val::operator==(const Val &other) const {
    return emlite_val_strictly_equals(v_, other.v_);
}

bool Val::operator!=(const Val &other) const {
    return !emlite_val_strictly_equals(v_, other.v_);
}

bool Val::operator>(const Val &other) const {
    return emlite_val_gt(v_, other.v_);
}

bool Val::operator>=(const Val &other) const {
    return emlite_val_gte(v_, other.v_);
}

bool Val::operator<(const Val &other) const {
    return emlite_val_lt(v_, other.v_);
}

bool Val::operator<=(const Val &other) const {
    return emlite_val_lte(v_, other.v_);
}

Console::Console() : Val(Val::global("console")) {}
} // namespace emlite