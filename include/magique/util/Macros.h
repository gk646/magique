#ifndef MAGIQUE_MACROS_H
#define MAGIQUE_MACROS_H

#include <magique/fwd.hpp>

template <typename... Args>
using EventFunc = void (magique::EntityScript::*)(Args...);

#define HANDLE_EVENT(eventType)                                                                                        \
    if constexpr (event == magique::EventType::eventType)                                                         \
        return &EntityScript::eventType;


#define CALL_MEMBER_FN(object, ptrToMember) ((object).*(ptrToMember))

#endif //MAGIQUE_MACROS_H