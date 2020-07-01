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
 * This is the header file for the CVODE serialization functions.
 *------------------------------------------------------------------
 */

#ifndef _CVODE_SERIALIZATION_H
#define _CVODE_SERIALIZATION_H

#ifdef __cplusplus  /* wrapper to enable C++ usage */
extern "C" {
#endif

#include <stdio.h>

#include <sundials/sundials_config.h>
#include <sundials/sundials_serialization.h>

/* Error codes returned from deserialization function */
#define CVODE_SERIALIZATION_INVALID_MAGIC_HEADER -1
#define CVODE_SERIALIZATION_INVALID_VERSION -2

#define CVODE_SERIALIZE_A(op, type, storageDataPtr, value, mem_size)\
  switch (op) {\
  case SUNDIALS_SERIALIZATION_OPERATION_SERIALIZE:\
    SERIALIZE(type, storageDataPtr, value)\
    break;\
  case SUNDIALS_SERIALIZATION_OPERATION_DESERIALIZE:\
    DESERIALIZE(type, storageDataPtr, value)\
    break;\
  case SUNDIALS_SERIALIZATION_OPERATION_SIZE:\
    mem_size += sizeof(type);\
    break;\
  }

#define CVODE_SERIALIZE_NVECTOR(op, storageDataPtr, valuePtr, mem_size)\
  switch (op) {\
  case SUNDIALS_SERIALIZATION_OPERATION_SERIALIZE:\
    SerializeNVector(storageDataPtr, valuePtr);\
    break;\
  case SUNDIALS_SERIALIZATION_OPERATION_DESERIALIZE:\
    DeSerializeNVector(storageDataPtr, valuePtr);\
    break;\
  case SUNDIALS_SERIALIZATION_OPERATION_SIZE:\
    mem_size += sizeof(size_t) + ((N_VectorContent_Serial)(valuePtr)->content)->length * sizeof(realtype);\
    break;\
  }

#define CVODE_SERIALIZE_DLSMAT(op, storageDataPtr, valuePtr, mem_size)\
  switch (op) {\
  case SUNDIALS_SERIALIZATION_OPERATION_SERIALIZE:\
    SerializeDlsMat(storageDataPtr, valuePtr);\
    break;\
  case SUNDIALS_SERIALIZATION_OPERATION_DESERIALIZE:\
    DeSerializeDlsMat(storageDataPtr, valuePtr);\
    break;\
  case SUNDIALS_SERIALIZATION_OPERATION_SIZE:\
    mem_size += SerializeDlsMatSize(valuePtr);\
    break;\
  }

/*
 * -----------------------------------------------------------------
 * Function : CVodeSerializationSize
 * -----------------------------------------------------------------
 * CVodeSerializationSize returns the size of the memory block in
 * bytes that is required to store the complete internal state of
 * the CVODE integrator (all data structures that are allocated by
 * CVODE/SUNDIALS functions themselves).
 * Returns size of memory in bytes required.
 * -----------------------------------------------------------------
 */
size_t CVodeSerializationSize(void *cvode_mem);

/*
 * -----------------------------------------------------------------
 * Function : CVodeSerialize
 * -----------------------------------------------------------------
 * CVodeSerialize copies the memory that defines the entire state of
 * CVODE integrator to a continuous memory array. The memory array
 * must have the size that is returned by CVodeSerializationSize().
 * The CVODE memory is not modified. The pointer to the storage location
 * is moved forward and points to the byte just after the last byte
 * of the serialized memory.
 * Returns size of copies memory in bytes.
 * -----------------------------------------------------------------
 */
size_t CVodeSerialize(void *cvode_mem, void ** storageDataPtr);

/*
 * -----------------------------------------------------------------
 * Function : CVodeDeserialize
 * -----------------------------------------------------------------
 * CVodeDeserialize copies the memory that defines the entire state of
 * CVODE integrator from a continuous memory array into the CVODE data
 * structure(s). The memory array must have the size that is returned
 * by CVodeSerializationSize() and the CVODE must be properly
 * initialized so that all NVectors etc. have the correct size.
 * The pointer to the storage location is moved forward and points to
 * the byte just after the last byte of the serialized memory.
 * Returns size of copies memory in bytes or error code.
 * -----------------------------------------------------------------
 */
size_t CVodeDeserialize(void *cvode_mem, void ** storageDataPtr);


size_t CVDlsSerializationSize(void *cvode_mem);
size_t CVDlsSerialize(void *cvode_mem, void ** storageDataPtr);
size_t CVDlsDeserialize(void *cvode_mem, void ** storageDataPtr);

size_t CVSpilsSerializationSize(void *cvode_mem);
size_t CVSpilsSerialize(void *cvode_mem, void ** storageDataPtr);
size_t CVSpilsDeserialize(void *cvode_mem, void ** storageDataPtr);


#ifdef __cplusplus
}
#endif

#endif
