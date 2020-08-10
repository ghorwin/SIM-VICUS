/*
 * -----------------------------------------------------------------
 * $Revision: 1.0 $
 * $Date: 2015/03/24 00:05:07 $
 * -----------------------------------------------------------------
 * Programmer(s): Jens Bastian, Andreas Nicolai
 * -----------------------------------------------------------------
 * Copyright (c) 2015, Technische Universität Dresden, Germany
 * For license details, see the LICENSE file.
 * -----------------------------------------------------------------
 * This is the header file for the SUNDIALS serialization functions.
 * -----------------------------------------------------------------
 */

#include <sundials/sundials_serialization.h>
#include <sundials/sundials_direct.h>
#include <nvector/nvector_serial.h>
#include <assert.h>
#include <memory.h>

void SerializeNVector(void **storageDataPtr, N_Vector vec) {
  N_VectorContent_Serial nvecSerial = (N_VectorContent_Serial)vec->content;
  SERIALIZE(size_t, *storageDataPtr, (size_t)nvecSerial->length);
  memcpy(*storageDataPtr, nvecSerial->data, sizeof(realtype)*(size_t)nvecSerial->length);
  *storageDataPtr = (char *)(*storageDataPtr) + sizeof(realtype)*(size_t)nvecSerial->length;
}


size_t SerializeDlsMatSize(DlsMat mat) {
  return sizeof(size_t) + (size_t)mat->ldata*sizeof(realtype);
}


void SerializeDlsMat(void **storageDataPtr, DlsMat mat) {
  SERIALIZE(size_t, *storageDataPtr, (size_t)mat->ldata); /* stored for sanity checks */
  memcpy(*storageDataPtr, mat->data, sizeof(realtype)*(size_t)mat->ldata);
  *storageDataPtr = (char *)(*storageDataPtr) + sizeof(realtype)*(size_t)mat->ldata;
}


void DeSerializeNVector(void **storageDataPtr, N_Vector vec) {
  size_t N;
  N_VectorContent_Serial nvecSerial = (N_VectorContent_Serial)vec->content;
  DESERIALIZE(size_t, *storageDataPtr, N);
  assert(N == (size_t)nvecSerial->length);
  memcpy(nvecSerial->data, *storageDataPtr, sizeof(realtype)*N);
  *storageDataPtr = (char *)(*storageDataPtr) + sizeof(realtype)*(size_t)nvecSerial->length;
}


void DeSerializeDlsMat(void **storageDataPtr, DlsMat mat) {
  size_t ldata;
  DESERIALIZE(size_t, *storageDataPtr, ldata);
  (void)ldata; // remove compiler warning about unused variable in release mode
  assert(ldata == (size_t)mat->ldata);
  memcpy(mat->data, *storageDataPtr, sizeof(realtype)*(size_t)mat->ldata);
  *storageDataPtr = (char *)(*storageDataPtr) + sizeof(realtype)*(size_t)mat->ldata;
}
