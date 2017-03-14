#include "array.h"

#include <stdexcept>

namespace clickhouse {

ColumnArray::ColumnArray(ColumnRef data)
    : Column(Type::CreateArray(data->Type()))
    , data_(data)
    , offsets_(std::make_shared<ColumnUInt64>())
{
}

void ColumnArray::AppendAsColumn(ColumnRef array) {
    if (!data_->Type()->IsEqual(array->Type())) {
        throw std::runtime_error(
            "can't append column of type " + array->Type()->GetName() + " "
            "to column type " + data_->Type()->GetName());
    }

    if (offsets_->Size() == 0) {
        offsets_->Append(array->Size());
    } else {
        offsets_->Append((*offsets_)[offsets_->Size() - 1] + array->Size());
    }

    data_->AppendFromColumn(array);
}

ColumnRef ColumnArray::GetAsColumn(size_t n) const {
    return data_->Slice(GetOffset(n), GetSize(n));
}

size_t ColumnArray::Size() const {
    return offsets_->Size();
}

bool ColumnArray::Load(CodedInputStream* input, size_t rows) {
    if (!offsets_->Load(input, rows)) {
        return false;
    }
    if (!data_->Load(input, (*offsets_)[rows - 1])) {
        return false;
    }
    return true;
}

void ColumnArray::Save(CodedOutputStream* output) {
    offsets_->Save(output);
    data_->Save(output);
}

}
