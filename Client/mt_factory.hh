/*
 * mt_factory.hh
 *
 *  Created on: Jul 31, 2012
 *      Author: nsoblath
 */

#ifndef MT_FACTORY_HH_
#define MT_FACTORY_HH_

#include "mt_singleton.hh"

#include <iostream>
#include <map>
#include <string>
#include <utility>

namespace Katydid
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
            virtual XBaseType* Create() const = 0;

    };

    template< class XBaseType, class XDerivedType >
    class registrar : public base_registrar< XBaseType >
    {
        public:
            registrar(const std::string& className);
            virtual ~registrar();

        protected:
            void Register(const std::string& className) const;

            XBaseType* Create() const;

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
            XBaseType* Create(const std::string& className);
            XBaseType* Create(const FactoryCIt& iter);

            void Register(const std::string& className, const base_registrar< XBaseType >* base_registrar);

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
    XBaseType* factory< XBaseType >::Create(const FactoryCIt& iter)
    {
        return iter->second->Create();
    }

    template< class XBaseType >
    XBaseType* factory< XBaseType >::Create(const std::string& className)
    {
        FactoryCIt it = fMap->find(className);
        if (it == fMap->end())
        {
            std::cerr << "Did not find factory for <" << className << ">." << std::endl;
            return NULL;
        }

        return it->second->Create();
    }

    template< class XBaseType >
    void factory< XBaseType >::Register(const std::string& className, const base_registrar< XBaseType >* base_registrar)
    {
        FactoryCIt it = fMap->find(className);
        if (it != fMap->end())
        {
            std::cerr << "Already have factory registered for <" << className << ">." << std::endl;
            return;
        }
        fMap->insert(std::pair< std::string, const base_registrar< XBaseType >* >(className, base_registrar));
        std::cout << "Registered a factory for class " << className << ", factory #" << fMap->size()-1 << std::endl;
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
    registrar< XBaseType, XDerivedType >::registrar(const std::string& className) :
            base_registrar< XBaseType >()
    {
        Register(className);
    }

    template< class XBaseType, class XDerivedType >
    registrar< XBaseType, XDerivedType >::~registrar()
    {}

    template< class XBaseType, class XDerivedType >
    void registrar< XBaseType, XDerivedType >::Register(const std::string& className) const
    {
        factory< XBaseType >::GetInstance()->Register(className, this);
        return;
    }

    template< class XBaseType, class XDerivedType >
    XBaseType* registrar< XBaseType, XDerivedType >::Create() const
    {
        return dynamic_cast< XBaseType* >(new XDerivedType());
    }

} /* namespace mantis */
#endif /* MT_FACTORY_HH_ */
