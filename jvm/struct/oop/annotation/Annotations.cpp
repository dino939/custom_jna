//
// Created by prone on 01.07.2026.
//

#include "Annotations.h"



 AnnotationArray* Annotations::class_annotations() const
{
    return _class_annotations;
}

 Array<AnnotationArray*>* Annotations::fields_annotations() const
{
    return _fields_annotations;
}

 AnnotationArray* Annotations::class_type_annotations() const
{
    return _class_type_annotations;
}

 Array<AnnotationArray*>* Annotations::fields_type_annotations() const
{
    return _fields_type_annotations;
}