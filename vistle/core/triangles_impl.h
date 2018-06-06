#ifndef TRIANGLES_IMPL_H
#define TRIANGLES_IMPL_H

namespace vistle {

template<class Archive>
void Triangles::Data::serialize(Archive &ar) {
   ar & V_NAME(ar, "base_coords", serialize_base<Base::Data>(ar, *this));
   ar & V_NAME(ar, "connection_list", cl);
}

} // namespace vistle
#endif

