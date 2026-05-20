#include "../../../include/backend/details/binmsg.hpp"

namespace sockmsg::backend::details::bmsg {

const Header* RawMessage::header() const
{
    if (data_.size() < sizeof(Header)) {
        return nullptr;
    }
    return reinterpret_cast<const Header*>(data_.data());
}

StringView RawMessage::body() const
{
    if (!is_correct()) {
        return {};
    }
    return data_.substr(sizeof(Header));
}

const void* RawMessage::body_ptr() const
{
    return body().data();
}

bool RawMessage::is_correct() const
{
    const Header* head = header();
    if (!head) {
        return false;
    }
    return data_.size() == sizeof(Header) + head->len;
}

}
