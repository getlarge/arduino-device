/*
 * Sensors.h
 *
 * Author:  Getlarge
 * Created: 2019-12-13
 */

#ifndef Sensors_h
#define Sensors_h

// struct Sensor {
//   char *name;
//   char *type;
//   char *value;
//   char *nativeId;
//   char *resourceId;
//   char *id;
// };

// const char* resources[][60] = {
//   { "5850", "digital_input"},
//   { "5850", "digital_input"},
//   { "5527", "text_input"},
// };

enum SensorKeys {
  S_OBJECT_ID = 1,
  S_NODE_ID = 2,
  S_SENSOR_ID = 3,
  S_RESOURCE_ID = 4
};
const int SensorKeysLength = 5;

template <typename Type> class Sensors {
public:
  Sensors(unsigned indexCount, unsigned keyCount)
      : nIndex(indexCount), nKey(keyCount), sensors(0) {
    if (indexCount > 0 && keyCount > 0) {
      sensors = new Type[indexCount * keyCount];
    }
  }

  ~Sensors() { delete[] sensors; }

  // indexing (parenthesis operator), two of them (for const correctness)
  // const Type &operator()(unsigned x, SensorKeys y) const { return
  // sensors[Type(y) * nIndex + x]; }
  // Type &operator()(unsigned x, SensorKeys y) { return sensors[Type(y) *
  // nIndex + x]; }

  Type get(unsigned x, SensorKeys y) { 
    return sensors[int(y) * nIndex + x]; 
  }
  
  const Type get(unsigned x, SensorKeys y) const {
    return sensors[int(y) * nIndex + x];
  }

  void set(unsigned x, SensorKeys y, Type value) {
    sensors[int(y) * nIndex + x] = value;
  }

  unsigned GetIndexCount() const { return nIndex; }
  unsigned GetKeysCount() const { return nKey; }

private:
  unsigned nIndex;
  unsigned nKey;
  Type *sensors;

  // to prevent unwanted copying:
  Sensors(const Sensors<Type> &);
  Sensors &operator=(const Sensors<Type> &);
};

template <typename Type> class Sensors<Type *> {
public:
  Sensors(unsigned indexCount, unsigned keyCount)
      : nIndex(indexCount), nKey(keyCount), sensors(0) {
    if (indexCount > 0 && keyCount > 0) {
      sensors = new Type *[indexCount * keyCount];
    }
  }

  ~Sensors() { delete sensors; }

  Type *get(unsigned x, SensorKeys y) { 
    return sensors[int(y) * nIndex + x]; 
  }

  const Type *get(unsigned x, SensorKeys y) const {
    return sensors[int(y) * nIndex + x];
  }

  void set(unsigned x, SensorKeys y, Type *value) {
    sensors[int(y) * nIndex + x] = new Type(*value);
  }

  unsigned GetIndexCount() const { return nIndex; }
  unsigned GetKeysCount() const { return nKey; }

private:
  unsigned nIndex;
  unsigned nKey;
  Type **sensors;

  // to prevent unwanted copying:
  Sensors(const Sensors<Type> &);
  Sensors &operator=(const Sensors<Type> &);
};

template <> void Sensors<char *>::set(unsigned x, SensorKeys y, char *value) {
  // Figure out how long the string in value is

  int length = 0;
  while (value[length] != '\0') {
    ++length;
  }
  ++length;

  sensors[int(y) * nIndex + x] = new char[length];
  for (int count = 0; count < length; ++count) {
    sensors[int(y) * nIndex + x][count] = value[count];
  }
}

template <> Sensors<char *>::~Sensors() { delete[] sensors; }

#endif
