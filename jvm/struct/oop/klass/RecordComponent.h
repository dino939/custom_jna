//
// Created by opencode on 04.08.2026.
//

#ifndef XDANTIDUMPMEM_RECORDCOMPONENT_H
#define XDANTIDUMPMEM_RECORDCOMPONENT_H

#include "../../../types.h"
#include "../annotation/Annotations.h"

// Simplified mirror of HotSpot's RecordComponent (src/hotspot/share/oops/recordComponent.hpp).
// Layout must match the JVM's memory layout on x64: two pointers followed by four u2 fields.
class RecordComponent {
public:
    AnnotationArray *_annotations;
    AnnotationArray *_type_annotations;
    u2 _name_index;
    u2 _descriptor_index;
    u2 _attributes_count;
    u2 _generic_signature_index;

    AnnotationArray *annotations() const { return _annotations; }
    AnnotationArray *type_annotations() const { return _type_annotations; }
    u2 name_index() const { return _name_index; }
    u2 descriptor_index() const { return _descriptor_index; }
    u2 attributes_count() const { return _attributes_count; }
    u2 generic_signature_index() const { return _generic_signature_index; }
};

#endif //XDANTIDUMPMEM_RECORDCOMPONENT_H
