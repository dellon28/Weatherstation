 #!/usr/bin/python3


#################################################################################################################################################
#                                                    CLASSES CONTAINING ALL THE APP FUNCTIONS                                                                                                    #
#################################################################################################################################################


class DB:

    def __init__(self,Config):

        from math import floor
        from os import getcwd
        from os.path import join
        from json import loads, dumps, dump
        from datetime import timedelta, datetime, timezone 
        from pymongo import MongoClient , errors, ReturnDocument
        from urllib import parse
        from urllib.request import  urlopen 
        from bson.objectid import ObjectId  
       
      
        self.Config                         = Config
        self.getcwd                         = getcwd
        self.join                           = join 
        self.floor                      	= floor 
        self.loads                      	= loads
        self.dumps                      	= dumps
        self.dump                       	= dump  
        self.datetime                       = datetime
        self.ObjectId                       = ObjectId 
        self.server		                = Config.DB_SERVER or ""
        self.port		                = Config.DB_PORT or ""
        self.raw_username                  = Config.DB_USERNAME or ""
        self.raw_password                  = Config.DB_PASSWORD or ""
        self.username                   	= parse.quote_plus(self.raw_username)
        self.password                   	= parse.quote_plus(self.raw_password)
        self.remoteMongo                	= MongoClient
        self.ReturnDocument                 = ReturnDocument
        self.PyMongoError               	= errors.PyMongoError
        self.BulkWriteError             	= errors.BulkWriteError  
        self.tls                            = False # MUST SET TO TRUE IN PRODUCTION

        self._missing_db_cfg = []
        if not self.raw_username:
            self._missing_db_cfg.append("DB_USERNAME")
        if not self.raw_password:
            self._missing_db_cfg.append("DB_PASSWORD")
        if not self.server:
            self._missing_db_cfg.append("DB_SERVER")
        if not self.port:
            self._missing_db_cfg.append("DB_PORT")

        if self._missing_db_cfg:
            print("MongoDB config missing:", ", ".join(self._missing_db_cfg))
            print("Create backend/.env and set DB_USERNAME, DB_PASSWORD, DB_SERVER, DB_PORT")


    def _get_remote_db(self):
        if self._missing_db_cfg:
            return None

        try:
            return self.remoteMongo(
                'mongodb://%s:%s@%s:%s' % (self.username, self.password, self.server, self.port),
                tls=self.tls,
                serverSelectionTimeoutMS=5000,
            )
        except Exception as e:
            print("MongoDB connect error", str(e))
            return None


    def __del__(self):
            # Delete class instance to free resources
            pass
 


    ####################
    # LAB 2 DATABASE UTIL FUNCTIONS  #
    ####################
    
    def addUpdate(self,data):
        '''ADD A NEW STORAGE LOCATION TO COLLECTION'''
        try:
            remotedb 	= self._get_remote_db()
            if remotedb is None:
                return False
            result      = remotedb.ELET2415.weather.insert_one(data)
        except Exception as e:
            msg = str(e)
            if "duplicate" not in msg:
                print("addUpdate error ",msg)
            return False
        else:                  
            return True
        
       

    def getAllInRange(self,start, end):
        '''RETURNS A LIST OF OBJECTS. THAT FALLS WITHIN THE START AND END DATE RANGE'''
        try:
            remotedb 	= self._get_remote_db()
            if remotedb is None:
                return []
            result      = list(remotedb.ELET2415.weather.find({"timestamp":{"$gte":int(start),"$lte":int(end)}}, {"_id":0}).sort("timestamp",1))
            #print(result)
            #print(end)
        except Exception as e:
            msg = str(e)
            print("getAllInRange error ",msg)            
        else:                  
            return result
        

    def humidityMMAR(self,start, end):
        '''RETURNS MIN, MAX, AVG AND RANGE FOR HUMIDITY. THAT FALLS WITHIN THE START AND END DATE RANGE'''
        try:
            remotedb 	= self._get_remote_db()
            if remotedb is None:
                return []
            result      = list(remotedb.ELET2415.weather.aggregate([{"$match": {"timestamp": {"$gte": int(start), "$lte": int(end)}}}, {"$group": {"_id": None, "humidity": {"$push": "$$ROOT.humidity"}}}, {"$project": {"_id": 0, "max": {"$max": "$humidity"}, "min": {"$min": "$humidity"},"avg": {"$avg": "$humidity"}, "range": {"$subtract": [{"$max": "$humidity"}, {"$min": "$humidity"}]}}}]))
        except Exception as e:
            msg = str(e)
            print("humidityMMAS error ",msg)            
        else:                  
            return result
        
    def temperatureMMAR(self,start, end):
        '''RETURNS MIN, MAX, AVG AND RANGE FOR TEMPERATURE. THAT FALLS WITHIN THE START AND END DATE RANGE'''
        try:
            remotedb 	= self._get_remote_db()
            if remotedb is None:
                return []
            result      = list(remotedb.ELET2415.weather.aggregate([{"$match": {"timestamp": {"$gte": int(start), "$lte": int(end)}}}, {"$group": {"_id": None, "temperature": {"$push": "$$ROOT.temperature"}}}, {"$project": {"_id": 0, "max": {"$max": "$temperature"}, "min": {"$min": "$temperature"},"avg": {"$avg": "$temperature"}, "range": {"$subtract": [{"$max": "$temperature"}, {"$min": "$temperature"}]}}}]))
        except Exception as e:
            msg = str(e)
            print("temperatureMMAS error ",msg)            
        else:                  
            return result


    def frequencyDistro(self,variable,start, end):
        '''RETURNS THE FREQUENCY DISTROBUTION FOR A SPECIFIED VARIABLE WITHIN THE START AND END DATE RANGE'''
        try:
            remotedb 	= self._get_remote_db()
            if remotedb is None:
                return []
            result      = list(remotedb.ELET2415.weather.aggregate([{"$match": {"timestamp": {"$gte": int(start), "$lte": int(end)}}}, {"$bucket": {"groupBy": "$" + variable, "boundaries": [0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100], "default": "outliers", "output": {"count": {"$sum": 1}}}}]))
        except Exception as e:
            msg = str(e)
            print("frequencyDistro error ",msg)            
        else:                  
            return result
        
    def getLatest(self):
        '''RETURNS THE MOST RECENT SENSOR READING'''
        try:
            remotedb 	= self._get_remote_db()
            if remotedb is None:
                return {}
            result      = remotedb.ELET2415.weather.find_one({}, sort=[("timestamp", -1)], projection={"_id": 0})
        except Exception as e:
            msg = str(e)
            print("getLatest error ",msg)            
        else:                  
            return result if result else {}
        
 



def main():
    from config import Config
    from time import time, ctime, sleep
    from math import floor
    from datetime import datetime, timedelta
    one = DB(Config)
 
 
    start = time() 
    end = time()
    print(f"completed in: {end - start} seconds")
    
if __name__ == '__main__':
    main()


    