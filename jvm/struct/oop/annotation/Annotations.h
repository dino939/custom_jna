//
// Created by prone on 01.07.2026.
//

#ifndef XDANTIDUMPMEM_ANNOTATIONS_H
#define XDANTIDUMPMEM_ANNOTATIONS_H
#include "../../../struct/oop/array/Array.h"

typedef Array<u1> AnnotationArray;
class Annotations
{
private:
    // If you add a new field that points to any metaspace object, you
    // must add this field to Annotations::metaspace_pointers_do().

    // Annotations for this class, or null if none.
    AnnotationArray*             _class_annotations;
    // Annotation objects (byte arrays) for fields, or null if no annotations.
    // Indices correspond to entries (not indices) in fields array.
    Array<AnnotationArray*>*     _fields_annotations;
    // Type annotations for this class, or null if none.
    AnnotationArray*             _class_type_annotations;
    Array<AnnotationArray*>*     _fields_type_annotations;

public:
    [[nodiscard]] AnnotationArray* class_annotations() const;

    [[nodiscard]] Array<AnnotationArray*>* fields_annotations() const;

    [[nodiscard]] AnnotationArray* class_type_annotations() const;

    [[nodiscard]] Array<AnnotationArray*>* fields_type_annotations() const;
};


#endif //XDANTIDUMPMEM_ANNOTATIONS_H