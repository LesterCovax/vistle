#include <module/module.h>
#include <core/triangles.h>
#include <core/indexed.h>
#include <core/normals.h>
#include <core/grid.h>
#include <core/database.h>

class WeldVertices: public vistle::Module {
  static const int NumPorts = 3;

 public:
   WeldVertices(const std::string &name, int moduleID, mpi::communicator comm);
   ~WeldVertices();

 private:
   virtual bool compute() override;
   vistle::Port *m_in[NumPorts], *m_out[NumPorts];
};

using namespace vistle;

WeldVertices::WeldVertices(const std::string &name, int moduleID, mpi::communicator comm)
    : vistle::Module("weld vertices and build indexed geometry", name, moduleID, comm)
{
    for (int i=0; i<NumPorts; ++i) {
        m_in[i] = createInputPort("data_in"+std::to_string(i));
        m_out[i] = createOutputPort("data_out"+std::to_string(i));
    }
}

WeldVertices::~WeldVertices()
{
}

struct Point {
    Point(Scalar x, Scalar y, Scalar z, Index v, const std::vector<const Scalar *> &floats)
        : floats(floats), x(x), y(y), z(z), v(v)
    {}
    const std::vector<const Scalar *> &floats;
    Scalar x, y, z;
    Index v;

    bool operator<(const Point &o) const {
        if (x < o.x)
            return true;
        if (x > o.x)
            return false;

        if (y < o.y)
            return true;
        if (y > o.y)
            return false;

        if (z < o.z)
            return true;
        if (z > o.z)
            return false;

        for (auto f: floats) {
            if (f[v] < f[o.v])
                return true;
            if (f[v] > f[o.v])
                return false;
        }

        return false;
    }
};


bool WeldVertices::compute() {

    Object::const_ptr oin[NumPorts];
    DataBase::const_ptr din[NumPorts];
    Object::const_ptr grid;
    Normals::const_ptr normals;
    std::vector<const Scalar *> floats;

    for (int i=0; i<NumPorts; ++i) {
        if (m_in[i]->isConnected()) {
            oin[i] = expect<Object>(m_in[i]);
            din[i] = DataBase::as(oin[i]);
            Object::const_ptr g;
            if (din[i]) {
                g = din[i]->grid();
                if (!g) {
                    din[i].reset();
                } else {
                    if (din[i]->mapping() == DataBase::Vertex) {
                        if (auto s = Vec<Scalar,1>::as(din[i])) {
                            floats.push_back(s->x());
                        } else if (auto v = Vec<Scalar,3>::as(din[i])) {
                            floats.push_back(s->x());
                            floats.push_back(s->y());
                            floats.push_back(s->z());
                        }
                    }
                }
            }
            if (!g && oin[i]->getInterface<GeometryInterface>())
                g = oin[i];
            if (grid) {
                if (*g != *grid) {
                    sendError("Grids on input ports do not match");
                    return true;
                }
            } else {
                auto gi = g->getInterface<GeometryInterface>();
                if (!gi) {
                    sendError("Input does not have a grid on port %s", m_in[i]->getName().c_str());
                    return true;
                }
                grid = g;
                normals = gi->normals();
                if (normals) {
                    floats.push_back(normals->x());
                    floats.push_back(normals->y());
                    floats.push_back(normals->z());
                }
            }
        }
    }

    Object::ptr ogrid;
    std::vector<Index> remap;
    std::map<Point, Index> indexMap;
    if (auto tri = Triangles::as(grid)) {

        Index num = tri->getNumCorners();
        const Index *cl = num>0 ? tri->cl() : nullptr;
        if (!cl)
            num = tri->getNumCoords();
        remap.reserve(num);
        const Scalar *x=tri->x(), *y=tri->y(), *z=tri->z();

        Triangles::ptr ntri(new Triangles(num, 0));
        Index *ncl = ntri->cl().data();
        auto &nx=ntri->x(), &ny=ntri->y(), &nz=ntri->z();

        if (cl) {
            Index count = 0;
            Index num = tri->getNumCorners();
            for (Index i=0; i<num; ++i) {
                Index v = cl[i];
                Point p(x[v], y[v], z[v], v, floats);
                auto &idx = indexMap[p];
                if (idx == 0) {
                    remap.push_back(v);
                    idx = ++count;
                    nx.push_back(x[v]);
                    ny.push_back(y[v]);
                    nz.push_back(z[v]);
                }
                ncl[i] = idx-1;
            }
            //sendInfo("found %d unique vertices among %d", count-1, num);
        } else {
            Index count = 0;
            Index num = tri->getNumCoords();
            for (Index v=0; v<num; ++v) {
                Point p(x[v], y[v], z[v], v, floats);
                auto &idx = indexMap[p];
                if (idx == 0) {
                    remap.push_back(v);
                    idx = ++count;
                    nx.push_back(x[v]);
                    ny.push_back(y[v]);
                    nz.push_back(z[v]);
                }
                ncl[v] = idx-1;
            }
            //sendInfo("found %d unique vertices among %d", count-1, num);
        }

        ogrid = ntri;
    } else if (auto idx = Indexed::as(grid)) {
        Index num = idx->getNumCorners();
        const Index *cl = num>0 ? idx->cl() : nullptr;
        if (!cl)
            num = idx->getNumCoords();
        const Scalar *x=idx->x(), *y=idx->y(), *z=idx->z();
        remap.reserve(num);

        auto nidx = idx->clone();
        nidx->resetArrays();
        nidx->resetCorners();
        nidx->cl().resize(num);

        Index *ncl = nidx->cl().data();
        auto &nx=nidx->x(), &ny=nidx->y(), &nz=nidx->z();

        if (cl) {
            Index count = 0;
            Index num = idx->getNumCorners();
            for (Index i=0; i<num; ++i) {
                Index v = cl[i];
                Point p(x[v], y[v], z[v], v, floats);
                auto &idx = indexMap[p];
                if (idx == 0) {
                    remap.push_back(v);
                    idx = ++count;
                    nx.push_back(x[v]);
                    ny.push_back(y[v]);
                    nz.push_back(z[v]);
                }
                ncl[i] = idx-1;
            }
            //sendInfo("found %d unique vertices among %d", count-1, num);
        } else {
            Index count = 0;
            Index num = idx->getNumCoords();
            for (Index v=0; v<num; ++v) {
                Point p(x[v], y[v], z[v], v, floats);
                auto &idx = indexMap[p];
                if (idx == 0) {
                    remap.push_back(v);
                    idx = ++count;
                    nx.push_back(x[v]);
                    ny.push_back(y[v]);
                    nz.push_back(z[v]);
                }
                ncl[v] = idx-1;
            }
            //sendInfo("found %d unique vertices among %d", count-1, num);
        }

        ogrid = nidx;
    }

    if (ogrid) {
        ogrid->copyAttributes(grid);
    }

    for (int i=0; i<NumPorts; ++i) {
        if (din[i]) {
            if (din[i]->mapping() == DataBase::Element) {
                auto dout = din[i]->clone();
                dout->setGrid(ogrid);
                addObject(m_out[i], dout);
            } else {
                auto dout = din[i]->clone();
                dout->resetArrays();
                dout->setSize(remap.size());
                if (auto si = Vec<Scalar>::as(Object::const_ptr(din[i]))) {
                    auto so = Vec<Scalar>::as(Object::ptr(dout));
                    assert(so);
                    const Scalar *x = si->x();
                    Scalar *ox = so->x().data();
                    for (Index i=0; i<remap.size(); ++i) {
                        ox[i] = x[remap[i]];
                    }
                } else if (auto vi = Vec<Scalar,3>::as(Object::const_ptr(din[i]))) {
                    auto vo = Vec<Scalar,3>::as(Object::ptr(dout));
                    assert(vo);
                    const Scalar *x=vi->x(), *y=vi->y(), *z=vi->z();
                    Scalar *ox=vo->x().data(), *oy=vo->y().data(), *oz=vo->z().data();
                    for (Index i=0; i<remap.size(); ++i) {
                        ox[i] = x[remap[i]];
                        oy[i] = y[remap[i]];
                        oz[i] = z[remap[i]];
                    }
                }
                dout->setGrid(ogrid);
                addObject(m_out[i], dout);
            }
        } else {
            addObject(m_out[i], ogrid);
        }
    }

    return true;
}

MODULE_MAIN(WeldVertices)