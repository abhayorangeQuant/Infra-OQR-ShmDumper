#include "iostream"
#include "sstream"
#include <chrono>
#include "DBConnector.h"

SymbolNameMap g_SymbolMap;
SymbolNameMap g_FnOMap;
TokenNameMap g_TokenMap;


  
bool DBConnector::insertTradeData(DBTrade *td)
{
	std::stringstream szQuery;
#ifndef TRADE_SIM
	szQuery << "INSERT INTO live_trade_table(strategy_name, client_name, trading_symbol, transaction_type, option_type, expiry_date, strike_price, exchange_id, price, quantity, time_stamp, local_order_id, exchange_order_id, exchange_trade_id, trade_date) values( '"
#else
	szQuery << "INSERT INTO sim_live_trade_table(strategy_name, client_name, trading_symbol, transaction_type, option_type, expiry_date, strike_price, exchange_id, price, quantity, time_stamp, local_order_id, exchange_order_id, exchange_trade_id, trade_date) values( '"
#endif
	<< td->m_szStrategyName << "','"
	<< td->m_szClientCode << "','"
	<< td->m_szScripCode << "','"
	<< td->m_cTransactionType << "','"
	<< td->m_szOptionType << "',"
	<< td->m_lExpiryDate << ","
	<< td->m_iStrikePrice << ","
	<< td->m_eExchangeType << ","
	<< td->m_dPrice << ","
	<< td->m_lQuantity << ",'"
	<< td->m_szTimeStamp << "',"
	<< td->m_ulLocalOrderID << ","
	<< td->m_ullExchangeOrderID << ","
	<< td->m_ulExchangeTradeID << ",'"
	<< td->m_szTradeDate << "')";

	pthread_mutex_lock(m_MySqlLock);
	if(mysql_query(&m_MySql, (char*)(szQuery.str().c_str())))
	{
		pthread_mutex_unlock(m_MySqlLock);
		std::cout << szQuery.str() << std::endl;
		std::cout << "DBError while inserting trade" << std::endl;
		return false;
	}
	//std::cout << "successfully inserted trade" << std::endl;
	pthread_mutex_unlock(m_MySqlLock);
	return true;
}

bool DBConnector::insertMinutelyData(DBMinutelyData *md)
{
	MYSQL_STMT* stmt;
	stmt= mysql_stmt_init(&m_MySql);
	const char* query = "INSERT INTO minutely_data_1 (epoch_time, exchange_id, trading_symbol, price_open, price_high, price_low, price_close, volume, open_interest) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)";

	if (mysql_stmt_prepare(stmt, query, strlen(query)) != 0)
    {
        cout << "Error: Could not prepare statement" << endl;
        exit(1);    
    }
    if (stmt == NULL)
    {
        cout << "Error: Could not initialize prepared statement" << endl;
        return 1;
    }
    
    MYSQL_BIND bind[9];
    memset(bind, 0, sizeof(bind));
	long long epoch_time = md->m_ulExchangeTimeStamp;
    int exchange_id = md->m_eExchangeType;
    char trading_symbol[255] = {0}; // initialize the array with all elements set to 0
    strcpy(trading_symbol, md->m_szScripCode);
    float price_open = md->m_dPriceOpen;
    float price_high = md->m_dPriceHigh;
    float price_low = md->m_dPriceLow;
    float price_close = md->m_dPriceClose;
    long long volume = md->m_lVolume;
    long long open_interest = md->m_lOpenInterest;
    bind[0].buffer_type = MYSQL_TYPE_LONGLONG;
    bind[0].buffer = &epoch_time;
    bind[1].buffer_type = MYSQL_TYPE_LONG;
    bind[1].buffer = &exchange_id;
    bind[2].buffer_type = MYSQL_TYPE_STRING;
    bind[2].buffer = trading_symbol;
    bind[2].buffer_length = sizeof(trading_symbol);  
    bind[3].buffer_type = MYSQL_TYPE_FLOAT;
    bind[3].buffer = &price_open;
    bind[4].buffer_type = MYSQL_TYPE_FLOAT;
    bind[4].buffer = &price_high;
    bind[5].buffer_type = MYSQL_TYPE_FLOAT;
    bind[5].buffer = &price_low;
    bind[6].buffer_type = MYSQL_TYPE_FLOAT;
    bind[6].buffer = &price_close;
    bind[7].buffer_type = MYSQL_TYPE_LONGLONG;
    bind[7].buffer = &volume;
    bind[8].buffer_type = MYSQL_TYPE_LONGLONG;
    bind[8].buffer = &open_interest;    
    mysql_stmt_bind_param(stmt, bind);
    auto epoch_time2 = std::chrono::system_clock::now().time_since_epoch().count();
    mysql_stmt_execute(stmt);
    auto epoch_time3 = std::chrono::system_clock::now().time_since_epoch().count();
    auto diff = epoch_time3 - epoch_time2; // calculate difference
    std::cout << "Difference in seconds: " << diff << std::endl;
    mysql_stmt_close(stmt);
    mysql_close(&m_MySql);

    return false;
}

// Please note the return pointer needs to be deteled by the caller
int DBConnector::fetchMinutelyDataRecords_records(DBMinutelyData** pMDList, int iExchangeType, char* szScripCode, int iRecords)
{
	std::stringstream szQuery;
	szQuery << "SELECT * FROM (SELECT * FROM minutely_data where exchange_id = " << iExchangeType << " and trading_symbol = '" << szScripCode << "'ORDER BY epoch_time DESC limit " << iRecords << ") sub ORDER BY epoch_time ASC";
	
	MYSQL_RES *result;
	MYSQL_ROW row;
	unsigned int iNumOfFields, iTotalRows, i, j;
	*pMDList = new DBMinutelyData[iRecords];
	pthread_mutex_lock(m_MySqlLock);
	if(mysql_query(&m_MySql, (char*)(szQuery.str().c_str())))
	{
		pthread_mutex_unlock(m_MySqlLock);
		std::cout << "DBError while fetching from minutely data" << std::endl;
		std::cout << szQuery.str() << std::endl;
	}
	else
	{
		result = mysql_store_result(&m_MySql);
		pthread_mutex_unlock(m_MySqlLock);

		iTotalRows = mysql_num_rows(result);
		iNumOfFields = mysql_num_fields(result);
		j = 0;
		while((row = mysql_fetch_row(result)))
		{
			for(i = 0; i < iNumOfFields; i++)
			{
				switch(i)
				{
					case 0:		(*pMDList)[j].m_ulExchangeTimeStamp = atol(row[i]);		break;
					case 1:		(*pMDList)[j].m_eExchangeType = (exchange_type)atoi(row[i]);	break;
					case 2:		strcpy((*pMDList)[j].m_szScripCode, row[i]);			break;
					case 3:		(*pMDList)[j].m_dPriceOpen = atof(row[i]);			break;
					case 4:		(*pMDList)[j].m_dPriceHigh = atof(row[i]);			break;
					case 5:		(*pMDList)[j].m_dPriceLow = atof(row[i]);			break;
					case 6:		(*pMDList)[j].m_dPriceClose = atof(row[i]);			break;
					case 7:		(*pMDList)[j].m_lVolume = atol(row[i]);				break;
					case 8:		(*pMDList)[j].m_lOpenInterest = atol(row[i]);			break;
					default:	std::cout << "DBWaring column > 9 minutely reco" << std::endl;	break;
				}
			}
			j++;
			//assert(j > iTotalRows);
		}
	}
	return iTotalRows;
}

#ifdef TRADE_SIM

int DBConnector::fetchMinutelyDataRecords_sim(DBMinutelyData** pMDList, int iExchangeType, char* szScripCode, unsigned long ulStartEpoch, int iRecords)
{
	static int iDayRecordCount = fetchMinDataSimulator(NULL, iExchangeType, ulStartEpoch, ulStartEpoch+86400, true);
	if(iDayRecordCount < 1)
	{
		std::cout << "Holiday detected" << std::endl;
		std::cout << "Done for the day" << std::endl; 
		// since this will be trading holiday lets just exit in backtest
		exit(0);
	}
	if(iRecords > 0)
	{
		std::stringstream szQuery;
		szQuery << "SELECT * FROM (SELECT * FROM minutely_data where trading_symbol = '" << szScripCode << "' and epoch_time <= " << ulStartEpoch << " and epoch_time >= " << ulStartEpoch - 2000000 << ") sub ORDER BY epoch_time ASC";
		std::cout << "DB sim fetch " << szQuery.str() << std::endl;
		MYSQL_RES *result;
		MYSQL_ROW row;
		unsigned int iNumOfFields, iTotalRows, i, j;
//		*pMDList = new DBMinutelyData[iRecords];
		pthread_mutex_lock(m_MySqlLock);
		if(mysql_query(&m_MySql, (char*)(szQuery.str().c_str())))
		{
			pthread_mutex_unlock(m_MySqlLock);
			std::cout << "DBError while fetching from minutely data" << std::endl;
			std::cout << szQuery.str() << std::endl;
		}
		else
		{
			result = mysql_store_result(&m_MySql);
			pthread_mutex_unlock(m_MySqlLock);

			iTotalRows = mysql_num_rows(result);
			*pMDList = new DBMinutelyData[iTotalRows];
			iNumOfFields = mysql_num_fields(result);
			j = 0;
			while((row = mysql_fetch_row(result)))
			{
				for(i = 0; i < iNumOfFields; i++)
				{
					switch(i)
					{
						case 0:		(*pMDList)[j].m_ulExchangeTimeStamp = atol(row[i]);		break;
						case 1:		(*pMDList)[j].m_eExchangeType = (exchange_type)atoi(row[i]);	break;
						case 2:		strcpy((*pMDList)[j].m_szScripCode, row[i]);			break;
						case 3:		(*pMDList)[j].m_dPriceOpen = atof(row[i]);			break;
						case 4:		(*pMDList)[j].m_dPriceHigh = atof(row[i]);			break;
						case 5:		(*pMDList)[j].m_dPriceLow = atof(row[i]);			break;
						case 6:		(*pMDList)[j].m_dPriceClose = atof(row[i]);			break;
						case 7:		(*pMDList)[j].m_lVolume = atol(row[i]);				break;
						case 8:		(*pMDList)[j].m_lOpenInterest = atol(row[i]);			break;
						default:	std::cout << "DBWaring column > 9 minutely reco" << std::endl;	break;
					}
				}
				j++;
				//assert(j > iTotalRows);
			}
		}
		return iTotalRows;
	}
	return 0;
}

#endif

// Please note the return pointer needs to be deteled by the caller
int DBConnector::fetchMinutelyDataRecords_epoch(DBMinutelyData** pMDList, int iExchangeType, char* szScripCode, unsigned long ulEpochDate, unsigned long ulEndEpochDate = 0)
{
	std::stringstream szQuery;
	if(ulEndEpochDate == 0)
	{
		szQuery << "SELECT * FROM minutely_data where exchange_id = " << iExchangeType << " and trading_symbol = '" << szScripCode << "' and epoch_time >= " << ulEpochDate << " ORDER BY epoch_time ASC";
	}
	else
	{
		szQuery << "SELECT * FROM minutely_data where exchange_id = " << iExchangeType << " and trading_symbol = '" << szScripCode << "' and epoch_time >= " << ulEpochDate << " and epoch_time < " << ulEndEpochDate << " ORDER BY epoch_time ASC";
	}
	std::cout << "Epoch query " << szQuery.str() << std::endl;
	MYSQL_RES *result;
	MYSQL_ROW row;
	unsigned int iNumOfFields, iTotalRows, i, j;
	pthread_mutex_lock(m_MySqlLock);
	if(mysql_query(&m_MySql, (char*)(szQuery.str().c_str())))
	{
		pthread_mutex_unlock(m_MySqlLock);
		std::cout << "DBError while fetching from minutely data" << std::endl;
		std::cout << szQuery.str() << std::endl;
	}
	else
	{
		result = mysql_store_result(&m_MySql);
		pthread_mutex_unlock(m_MySqlLock);

		iTotalRows = mysql_num_rows(result);
		*pMDList = new DBMinutelyData[iTotalRows];
		iNumOfFields = mysql_num_fields(result);
		j = 0;
		while((row = mysql_fetch_row(result)))
		{
			for(i = 0; i < iNumOfFields; i++)
			{
				switch(i)
				{
					case 0:		(*pMDList)[j].m_ulExchangeTimeStamp = atol(row[i]);		break;
					case 1:		(*pMDList)[j].m_eExchangeType = (exchange_type)atoi(row[i]);	break;
					case 2:		strcpy((*pMDList)[j].m_szScripCode, row[i]);			break;
					case 3:		(*pMDList)[j].m_dPriceOpen = atof(row[i]);			break;
					case 4:		(*pMDList)[j].m_dPriceHigh = atof(row[i]);			break;
					case 5:		(*pMDList)[j].m_dPriceLow = atof(row[i]);			break;
					case 6:		(*pMDList)[j].m_dPriceClose = atof(row[i]);			break;
					case 7:		(*pMDList)[j].m_lVolume = atol(row[i]);				break;
					case 8:		(*pMDList)[j].m_lOpenInterest = atol(row[i]);			break;
					default:	std::cout << "DBWarning column > 9 minutely epoc" << std::endl;	break;
				}
			}
			j++;
			//assert(j > iTotalRows);
		}
	}
	return iTotalRows;
}

#ifdef TRADE_SIM
unsigned long DBConnector::fetchMinDataSimulator(DBMinutelyData** pMDList, int iExchangeType, unsigned long ulStartEpoch, unsigned long ulEndEpoch, bool bCountOnly)
{
	// so that query happens only once since this is a costly query
	static int iStaticQueryCount = 0;
	static MYSQL_RES *result = NULL;
        
	unsigned int iNumOfFields, i, j;
	unsigned long iTotalRows = 0;
	MYSQL_ROW row;

	if(iStaticQueryCount == 0)
	{
		std::stringstream szQuery;
		szQuery << "SELECT * FROM minutely_data where epoch_time >= " << ulStartEpoch << " and epoch_time <= " << ulEndEpoch << " ORDER BY epoch_time ASC";
		std::cout << (char*)(szQuery.str().c_str()) << std::endl;
		pthread_mutex_lock(m_MySqlLock);
		if(mysql_query(&m_MySql, (char*)(szQuery.str().c_str())))
		{
			pthread_mutex_unlock(m_MySqlLock);
			std::cout << "DBError while fetching from minutely data" << std::endl;
			std::cout << szQuery.str() << std::endl;
		}
		else
		{
			result = mysql_store_result(&m_MySql);
			pthread_mutex_unlock(m_MySqlLock);
		}
		iStaticQueryCount = 1;
	}
	if(result != NULL)
	{
		if(bCountOnly)
		{
			iTotalRows = mysql_num_rows(result);
		}
		else
		{
			iTotalRows = mysql_num_rows(result);
			*pMDList = new DBMinutelyData[iTotalRows];
			iNumOfFields = mysql_num_fields(result);
			j = 0;
			while((row = mysql_fetch_row(result)))
			{
				for(i = 0; i < iNumOfFields; i++)
				{
					switch(i)
					{
						case 0:         (*pMDList)[j].m_ulExchangeTimeStamp = atol(row[i]);             break;
						case 1:         (*pMDList)[j].m_eExchangeType = (exchange_type)atoi(row[i]);    break;
						case 2:         strcpy((*pMDList)[j].m_szScripCode, row[i]);                    break;
						case 3:         (*pMDList)[j].m_dPriceOpen = atof(row[i]);                      break;
						case 4:         (*pMDList)[j].m_dPriceHigh = atof(row[i]);                      break;
						case 5:         (*pMDList)[j].m_dPriceLow = atof(row[i]);                       break;
						case 6:         (*pMDList)[j].m_dPriceClose = atof(row[i]);                     break;
						case 7:         (*pMDList)[j].m_lVolume = atol(row[i]);                         break;
						case 8:         (*pMDList)[j].m_lOpenInterest = atol(row[i]);                   break;
						default:        std::cout << "DBWarning column > 9 minutely epoc" << std::endl; break;
					}
				}
				j++;
				//assert(j > iTotalRows);
			}
		}
	}
        return iTotalRows;
};

#endif

// Please note the return pointer needs to be deteled by the caller
int DBConnector::fetchTradeRecords(DBTrade** pTradeList, char* m_szClientCode, char* m_szStrategyName)
{
	std::stringstream szQuery;
	std::stringstream szQuery_cf;
#ifndef TRADE_SIM
	if(m_szClientCode != NULL && m_szClientCode[0] != '\0')
	{
		szQuery << "SELECT * FROM live_trade_table where client_name = '" << m_szClientCode << "' ORDER BY time_stamp ASC";
	}
	else if(m_szStrategyName != NULL && m_szStrategyName[0] != '\0')
	{
		szQuery << "SELECT * FROM live_trade_table where strategy_name = '" << m_szStrategyName << "' ORDER BY time_stamp ASC";
	}
#else
	if(m_szClientCode != NULL && m_szClientCode[0] != '\0')
	{
		szQuery << "SELECT * FROM sim_live_trade_table where client_name = '" << m_szClientCode << "' ORDER BY time_stamp ASC";
	}
	else if(m_szStrategyName != NULL && m_szStrategyName[0] != '\0')
	{
		szQuery << "SELECT * FROM sim_live_trade_table where strategy_name = '" << m_szStrategyName << "' ORDER BY time_stamp ASC";
	}
#endif
	MYSQL_RES *result;
	MYSQL_ROW row;
	unsigned int iNumOfFields = 0, iTotalRows = 0, i, j;
	pthread_mutex_lock(m_MySqlLock);
	if(mysql_query(&m_MySql, (char*)(szQuery.str().c_str())))
	{
		pthread_mutex_unlock(m_MySqlLock);
		std::cout << "DBError while fetching live trades" << std::endl;
		std::cout << szQuery.str() << std::endl;
		iTotalRows = 0;
	}
	else
	{
		result = mysql_store_result(&m_MySql);
		iTotalRows = mysql_num_rows(result);
		iNumOfFields = mysql_num_fields(result);
		pthread_mutex_unlock(m_MySqlLock);
		*pTradeList = new DBTrade[iTotalRows];
		j = 0;
		while((row = mysql_fetch_row(result)))
		{
			for(i = 0; i < iNumOfFields; i++)
			{
				switch(i)
				{
					case 0:		strcpy((*pTradeList)[j].m_szStrategyName, row[i]);	break;
					case 1:		strcpy((*pTradeList)[j].m_szClientCode, row[i]);	break;
					case 2:		strcpy((*pTradeList)[j].m_szScripCode, row[i]);		break;
					case 3:		(*pTradeList)[j].m_cTransactionType = row[i][0];	break;
					case 4:		break; //instrument name
					case 5:		strcpy((*pTradeList)[j].m_szOptionType, row[i]);	break;
					case 6:		(*pTradeList)[j].m_lExpiryDate = atol(row[i]);		break;
					case 7:		(*pTradeList)[j].m_iStrikePrice = atoi(row[i]);		break;
					case 8:		(*pTradeList)[j].m_eExchangeType = (exchange_type)atoi(row[i]);	break;
					case 9:		(*pTradeList)[j].m_dPrice = atof(row[i]);		break;
					case 10:	(*pTradeList)[j].m_lQuantity = atol(row[i]);		break;
					case 11:	strcpy((*pTradeList)[j].m_szTimeStamp, row[i]);		break;
					case 12:	(*pTradeList)[j].m_ulLocalOrderID = atol(row[i]);	break;
					case 13:	sscanf(row[i], "%llu", &((*pTradeList)[j].m_ullExchangeOrderID));	break;
					case 14:	(*pTradeList)[j].m_ulExchangeTradeID = atol(row[i]);	break;
					case 15:	strcpy((*pTradeList)[j].m_szTradeDate, row[i]);		break;
					default:	std::cout << "DBWarning column > 15 trade data" << std::endl;	break;
				}
			}
			j++;
			//assert(j > iTotalRows);
		}
	}
	return iTotalRows;
}

int DBConnector::fetchContractData(char* szSymbol, int iExchangeType)
{
	std::stringstream szQuery;
	szQuery << "SELECT * FROM contract_table where exchange_id =  " << iExchangeType << " and trading_symbol like '" << szSymbol << "%' ;";
	std::cout << "DBQuery|" << szQuery.str() << std::endl;

	MYSQL_RES *result;
	MYSQL_ROW row;
	unsigned int iTotalRows;

	if(mysql_query(&m_MySql, (char*)(szQuery.str().c_str())))
	{
		std::cout << "DBError while fetching from contract data" << std::endl;
		std::cout << szQuery.str() << std::endl;
	}
	else
	{
		ContractData data;
		result = mysql_store_result(&m_MySql);
		iTotalRows = mysql_num_rows(result);
		while((row = mysql_fetch_row(result)))
		{
			data.clear();
			strcpy(data.m_szScripCode, row[0]);
			strcpy(data.m_szOptionType, row[2]);
			data.m_fStrikePrice = atof(row[3]);
			data.m_iToken = atol(row[5]);
			data.m_iExchangeType = atoi(row[6]);
			if(data.m_iExchangeType == (int)MCX)
			{
				data.m_iLotSize = atoi(row[13]);
			}
			else if(data.m_iExchangeType == (int)NSE_CDS)
			{
				data.m_iLotSize = atoi(row[7])/1000;
			}
			else
			{
				data.m_iLotSize = atoi(row[7]);
			}
			data.m_iTickSize = atoi(row[8]);
			data.m_fPriceFactor = atof(row[9]);
			data.m_fPriceMultiplier = atof(row[10]);

			// store in a map
			g_FnOMap[std::string(data.m_szScripCode)] = data;
		}
	}
	return iTotalRows;
}

int DBConnector::fetchTradedValue(int iExchangeType)
{
        std::stringstream szQuery;
	if(iExchangeType == (int)NSE_CASH)
	{
		szQuery << "SELECT trading_symbol , sum(value_traded ) from bhav_table as bts inner join (select distinct(trade_date) as ddates from bhav_table where exchange_id = 1 order by ddates desc limit 5) as dts on bts.trade_date = dts.ddates where  bts.exchange_id = 1  and bts.trading_symbol like '%-EQ' group by trading_symbol ;";
	}
	else
	{
        	szQuery << "SELECT trading_symbol , sum(value_traded ) from bhav_table as bts inner join (select distinct(trade_date) as ddates from bhav_table where exchange_id = 1 order by ddates desc limit 5) as dts on bts.trade_date = dts.ddates where  bts.exchange_id =  " << iExchangeType << "group by trading_symbol;";
	}
        std::cout << "DBQuery|" << szQuery.str() << std::endl;

        MYSQL_RES *result;
        MYSQL_ROW row;
        unsigned int iTotalRows;

        if(mysql_query(&m_MySql, (char*)(szQuery.str().c_str())))
        {
                std::cout << "DBError while fetching from contract data" << std::endl;
                std::cout << szQuery.str() << std::endl;
        }
        else
        {
                result = mysql_store_result(&m_MySql);
                iTotalRows = mysql_num_rows(result);
                while((row = mysql_fetch_row(result)))
		{
			SymbolNameMapIter iter = g_SymbolMap.find(std::string(row[0]));
			if(iter != g_SymbolMap.end())
			{
				iter->second.m_dTradedValue = atol(row[1])/5;
				g_TokenMap[iter->second.m_iToken].m_dTradedValue = iter->second.m_dTradedValue;
			}

		}
		
        }
        return iTotalRows;
}

int DBConnector::fetchContractDataPriceBands(int iExchangeType)
{
        std::stringstream szQuery;
        szQuery << "SELECT * FROM contract_table where exchange_id =  " << iExchangeType << ";";
        std::cout << "DBQuery|" << szQuery.str() << std::endl;

        MYSQL_RES *result;
        MYSQL_ROW row;
        unsigned int iTotalRows;

        if(mysql_query(&m_MySql, (char*)(szQuery.str().c_str())))
        {
                std::cout << "DBError while fetching from contract data" << std::endl;
                std::cout << szQuery.str() << std::endl;
        }
        else
        {
                ContractData data;
                result = mysql_store_result(&m_MySql);
                iTotalRows = mysql_num_rows(result);
                while((row = mysql_fetch_row(result)))
                {
                        data.clear();
                        strcpy(data.m_szScripCode, row[0]);
                        strcpy(data.m_szOptionType, row[2]);
                        data.m_fStrikePrice = atof(row[3]);
                        data.m_iToken = atol(row[5]);
                        data.m_iExchangeType = atoi(row[6]);
                        if(data.m_iExchangeType == (int)MCX)
                        {
                                data.m_iLotSize = atoi(row[13]);
                        }
                        else if(data.m_iExchangeType == (int)NSE_CDS)
                        {
                                data.m_iLotSize = atoi(row[7])/1000;
                        }
                        else
                        {
                                data.m_iLotSize = atoi(row[7]);
                        }
                        data.m_iTickSize = atoi(row[8]);
                        data.m_fPriceFactor = atof(row[9]);
                        data.m_fPriceMultiplier = atof(row[10]);

                        // store in a map
                        g_SymbolMap[std::string(data.m_szScripCode)] = data;
			// token map is used by MD hence price bands are not updated
			g_TokenMap[data.m_iToken] = data;
                }
		if(iExchangeType == (int)NSE_CASH)
		{
			szQuery.str(std::string(""));
			szQuery.clear();
			szQuery << "SELECT * FROM price_band;";
			std::cout << "DBQuery|" << szQuery.str() << std::endl;

			if(mysql_query(&m_MySql, (char*)(szQuery.str().c_str())))
			{
				std::cout << "DBError while fetching from contract data" << std::endl;
				std::cout << szQuery.str() << std::endl;
			}
			else
			{
				result = mysql_store_result(&m_MySql);
				iTotalRows = mysql_num_rows(result);
				while((row = mysql_fetch_row(result)))
				{
					SymbolNameMapIter iter = g_SymbolMap.find(std::string(row[0]));
					if(iter != g_SymbolMap.end())
					{
						iter->second.m_iPriceBand = atoi(row[1]);
						iter->second.m_fClosePrice = atof(row[2]);
					}
				}
			}
			// for now adding for EQ only
			fetchTradedValue(iExchangeType);
		}
        }
        return iTotalRows;
}

DBConnector::DBConnector(SettingsReader* reader)
{ 
	MYSQL_ROW row; 
	MYSQL_RES *result; 

	unsigned int num_fields; 
	unsigned int i; 

	std::string szIP = reader->GetString("db", "Host", "localhost");
        std::string szUserName = reader->GetString("db", "Username", "root");
        std::string szPassword = reader->GetString("db", "Password", "mysql");
        std::string szDatabase = reader->GetString("db", "Database", "infra");

	m_MySqlLock = new pthread_mutex_t();
	pthread_mutex_init(m_MySqlLock, NULL);
	mysql_init(&m_MySql); 
    
	//if(!mysql_real_connect(&m_MySql,"localhost","root","mysql","storage",0,NULL,0)) 
	if(!mysql_real_connect(&m_MySql, szIP.c_str(), szUserName.c_str(), szPassword.c_str(), szDatabase.c_str(), 0, NULL, 0))
	{
		std::cerr << "DBError Failed to connect to database: Error: " << mysql_error(&m_MySql) << std::endl; 
		// since we are calling this in constructor there will be no error code return
		// hence better to exit incase of connection failure
		exit(1);
	} 
   
         
	std::cout << "DBInfo Connected to Database" << std::endl;
}
