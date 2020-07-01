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
 *------------------------------------------------------------------
 */

#ifndef _SUNDIALS_SERIALIZATION_H
#define _SUNDIALS_SERIALIZATION_H

#include <memory.h>

#ifdef __cplusplus  /* wrapper to enable C++ usage */
extern "C" {
#endif

#ifndef _SUNDIALS_CONFIG_H
#define _SUNDIALS_CONFIG_H
#include <sundials/sundials_config.h>
#endif

#include <sundials/sundials_types.h>

#include <sundials/sundials_dense.h>
#include <nvector/nvector_serial.h>

/* Operation constant: Serialization. */
#define SUNDIALS_SERIALIZATION_OPERATION_SERIALIZE 0
/* Operation constant: De-Serialization. */
#define SUNDIALS_SERIALIZATION_OPERATION_DESERIALIZE 1
/* Operation constant: Size calculation. */
#define SUNDIALS_SERIALIZATION_OPERATION_SIZE 2


#define SERIALIZE(type, storageDataPtr, value)\
{\
  *(type *)(storageDataPtr) = (value);\
  (storageDataPtr) = (char *)(storageDataPtr) + sizeof(type);\
}

#define DESERIALIZE(type, storageDataPtr, value)\
{\
  (value) = *(type *)(storageDataPtr);\
  (storageDataPtr) = (char *)(storageDataPtr) + sizeof(type);\
}

/*
 * -----------------------------------------------------------------
 * Function : SerializeNVector
 * -----------------------------------------------------------------
 * Stores the N_Vector in the memory position pointed to by
 * storageDataPtr and moves said pointer forward by the size of
 * a size_t plus the number of the elements times the size of
 * realtype.
 *
 * The function does not return anything.
 * -----------------------------------------------------------------
 */
void SerializeNVector(void ** storageDataPtr, N_Vector vec);

/*
 * -----------------------------------------------------------------
 * Function : SerializeDlsMatSize
 * -----------------------------------------------------------------
 * Returns size in bytes that is needed to store content of DlsMat
 * structure.
 * -----------------------------------------------------------------
 */
size_t SerializeDlsMatSize(DlsMat mat);

/*
 * -----------------------------------------------------------------
 * Function : SerializeDlsMat
 * -----------------------------------------------------------------
 * Stores the DlsMat in the memory position pointed to by
 * storageDataPtr and moves said pointer forward.
 *
 * The function does not return anything.
 * -----------------------------------------------------------------
 */
void SerializeDlsMat(void ** storageDataPtr, DlsMat mat);

/*
 * -----------------------------------------------------------------
 * Function : DeSerializeNVector
 * -----------------------------------------------------------------
 * Retrieves the N_Vector from the memory position pointed to by
 * storageDataPtr and moves said pointer forward by the size of
 * a size_t plus the number of the elements times the size of
 * realtype.
 *
 * It is assumed that N_Vector vec has already the right dimension
 * and allocated data structure.
 *
 * The function does not return anything.
 * -----------------------------------------------------------------
 */
void DeSerializeNVector(void ** storageDataPtr, N_Vector vec);

/*
 * -----------------------------------------------------------------
 * Function : DeSerializeDlsMat
 * -----------------------------------------------------------------
 * Retrieves the DlsMat in the memory position pointed to by
 * storageDataPtr and moves said pointer forward.
 *
 * It is assumed that DlsMat has already the right dimensions
 * and allocated data structure.
 *
 * The function does not return anything.
 * -----------------------------------------------------------------
 */
void DeSerializeDlsMat(void ** storageDataPtr, DlsMat mat);

#ifdef __cplusplus
}
#endif

#endif
