/*
* Copyright (C) 2013 Nivis LLC.
* Email:   opensource@nivis.com
* Website: http://www.nivis.com
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, version 3 of the License.
* 
* Redistribution and use in source and binary forms must retain this
* copyright notice.

* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*/


#ifndef ENTITIESSTORE_H_
#define ENTITIESSTORE_H_




namespace hart7 {
namespace hostapp {


class IFactoryDal;



/*
 * Singleton pattern
 */
class EntitiesStore
{
public:
	enum StoreType
	{
		StoreType_Mem = 1,
		StoreType_SQLite,
		StoreType_MySQL,
		StoreType_Virt
	};

private:
	EntitiesStore();
	~EntitiesStore();
	EntitiesStore(const EntitiesStore&);
	EntitiesStore(const std::string &strParams, StoreType st);


//instance
public:
	static EntitiesStore* GetInstance(const std::string &strParams, StoreType st=StoreType_Virt);
	static void DeleteInstance();

//get
public:
	IFactoryDal* GetFactoryDal() {return m_pFactoryDAL;}

//
private:
	IFactoryDal *m_pFactoryDAL;


};

}
}

#endif


