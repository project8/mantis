/*
 * mt_factory.hh
 *
 *  created on: Jul 31, 2012
 *      Author: nsoblath
 */

#ifndef MT_FACTORY_HH_
#define MT_FACTORY_HH_

#include "mt_singleton.hh"

#include <iostream>
#include <map>
#include <string>
#include <utility>

namespace mantis
{
    template< class XBaseType >
    class factory;

    template< class XBaseType >
    class base_registrar
    {
        public:
            base_registrar() {}
            virtual ~base_registrar() {}

        public:
            friend class factory< XBaseType >;

        protected:
            virtual XBaseType* create() const = 0;

    };

    template< class XBaseType, class XDerivedType >
    class registrar : public base_registrar< XBaseType >
    {
        public:
            registrar(const std::string& a_class_name);
            virtual ~registrar();

        protected:
            void register_class(const std::string& a_class_name) const;

            XBaseType* create() const;

    };


    template< class XBaseType >
    class factory : public singleton< factory< XBaseType > >
    {
        public:
            typedef std::map< std::string, const base_registrar< XBaseType >* > FactoryMap;
            typedef typename FactoryMap::value_type FactoryEntry;
            typedef typename FactoryMap::iterator FactoryIt;
            typedef typename FactoryMap::const_iterator FactoryCIt;

        public:
            XBaseType* create(const std::string& a_class_name);
            XBaseType* create(const FactoryCIt& iter);

            void register_class(const std::string& a_class_name, const base_registrar< XBaseType >* base_registrar);

            FactoryCIt GetFactoryMapBegin() const;
            FactoryCIt GetFactoryMapEnd() const;

        protected:
            FactoryMap* fMap;


        protected:
            friend class singleton< factory >;
            friend class destroyer< factory >;
            factory();
            ~factory();
    };

    template< class XBaseType >
    XBaseType* factory< XBaseType >::create(const FactoryCIt& iter)
    {
        return iter->second->create();
    }

    template< class XBaseType >
    XBaseType* factory< XBaseType >::create(const std::string& a_class_name)
    {
        FactoryCIt it = fMap->find(a_class_name);
        if (it == fMap->end())
        {
            std::cerr << "Did not find factory for <" << a_class_name << ">." << std::endl;
            return NULL;
        }

        return it->second->create();
    }

    template< class XBaseType >
    void factory< XBaseType >::register_class(const std::string& a_class_name, const base_registrar< XBaseType >* base_registrar)
    {
        FactoryCIt it = fMap->find(a_class_name);
        if (it != fMap->end())
        {
            std::cerr << "Already have factory register_classed for <" << a_class_name << ">." << std::endl;
            return;
        }
        fMap->insert(std::pair< std::string, const base_registrar< XBaseType >* >(a_class_name, base_registrar));
        std::cout << "register_classed a factory for class " << a_class_name << ", factory #" << fMap->size()-1 << std::endl;
    }

    template< class XBaseType >
    factory< XBaseType >::factory() :
        fMap(new FactoryMap())
    {}

    template< class XBaseType >
    factory< XBaseType >::~factory()
    {
        delete fMap;
    }

    template< class XBaseType >
    typename factory< XBaseType >::FactoryCIt factory< XBaseType >::GetFactoryMapBegin() const
    {
        return fMap->begin();
    }

    template< class XBaseType >
    typename factory< XBaseType >::FactoryCIt factory< XBaseType >::GetFactoryMapEnd() const
    {
        return fMap->end();
    }




    template< class XBaseType, class XDerivedType >
    registrar< XBaseType, XDerivedType >::registrar(const std::string& a_class_name) :
            base_registrar< XBaseType >()
    {
        register_class(a_class_name);
    }

    template< class XBaseType, class XDerivedType >
    registrar< XBaseType, XDerivedType >::~registrar()
    {}

    template< class XBaseType, class XDerivedType >
    void registrar< XBaseType, XDerivedType >::register_class(const std::string& a_class_name) const
    {
        factory< XBaseType >::GetInstance()->register_class(a_class_name, this);
        return;
    }

    template< class XBaseType, class XDerivedType >
    XBaseType* registrar< XBaseType, XDerivedType >::create() const
    {
        return dynamic_cast< XBaseType* >(new XDerivedType());
    }

} /* namespace mantis */
#endif /* MT_FACTORY_HH_ */
