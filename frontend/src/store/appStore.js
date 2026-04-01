import {defineStore} from 'pinia'
import {ref} from 'vue'


export const useAppStore =  defineStore('app', ()=>{

    /*  
    The composition API way of defining a Pinia store
    ref() s become state properties
    computed() s become getters
    function() s become actions  
    */ 

    // STATES   


    // ACTIONS
    const getAllInRange = async (start,end)=>{
        // Fetch historical sensor readings between two Unix timestamps.
        const controller = new AbortController();
        const signal = controller.signal;
        const id = setTimeout(()=>{controller.abort()},60000);
        const URL = `/api/weather/get/${start}/${end}`;
        try {
            const response = await fetch(URL,{ method: 'GET', signal: signal });
            if (response.ok){
                const data = await response.json();
                let keys = Object.keys(data);
                if(keys.includes("status")){
                    if(data["status"] == "found" ){
                        // console.log(data["data"] )
                        return data["data"];
                    }
                    if(data["status"] == "failed"){
                        console.log("getAllInRange returned no data");
                    }
                }
            }
            else{
                const data = await response.text();
                console.log(data);
            }
        }
        catch(err){
            console.error('getAllInRange error: ', err.message);
        }
        return []
    }

    const getTemperatureMMAR = async (start,end)=>{
        // Fetch min, max, average, and range for temperature in a time window.
        const controller = new AbortController();
        const signal = controller.signal;
        const id = setTimeout(()=>{controller.abort()},60000);
        const URL = `/api/mmar/temperature/${start}/${end}`;
        try {
            const response = await fetch(URL,{ method: 'GET', signal: signal });
            if (response.ok){
                const data = await response.json();
                let keys = Object.keys(data);
                if(keys.includes("status")){
                    if(data["status"] == "found" ){
                        // console.log(data["data"] )
                        return data["data"];
                    }
                    if(data["status"] == "failed"){
                        console.log("getTemperatureMMAR returned no data");
                    }
                }
            }
            else{
                const data = await response.text();
                console.log(data);
            }
        }
        catch(err){
            console.error('getTemperatureMMAR error: ', err.message);
        }
        return []
    }

    const getHumidityMMAR = async (start,end)=>{
        // Fetch min, max, average, and range for humidity in a time window.
        const controller = new AbortController();
        const signal = controller.signal;
        const id = setTimeout(()=>{controller.abort()},60000);
        const URL = `/api/mmar/humidity/${start}/${end}`;
        try {
            const response = await fetch(URL,{ method: 'GET', signal: signal });
            if (response.ok){
                const data = await response.json();
                let keys = Object.keys(data);
                if(keys.includes("status")){
                    if(data["status"] == "found" ){
                        // console.log(data["data"] )
                        return data["data"];
                    }
                    if(data["status"] == "failed"){
                        console.log("getHumidityMMAR returned no data");
                    }
                }
            }
            else{
                const data = await response.text();
                console.log(data);
            }
        }
        catch(err){
            console.error('getHumidityMMAR error: ', err.message);
        }
        return []
    }

    const getFreqDistro = async (variable,start,end)=>{
        // Fetch a frequency distribution for a selected variable.
        const controller = new AbortController();
        const signal = controller.signal;
        const id = setTimeout(()=>{controller.abort()},60000);
        const URL = `/api/frequency/${variable}/${start}/${end}`;
        try {
            const response = await fetch(URL,{ method: 'GET', signal: signal });
            if (response.ok){
                const data = await response.json();
                let keys = Object.keys(data);
                if(keys.includes("status")){
                    if(data["status"] == "found" ){
                        // console.log(data["data"] )
                        return data["data"];
                    }
                    if(data["status"] == "failed"){
                        console.log("getFreqDistro returned no data");
                    }
                }
            }
            else{
                const data = await response.text();
                console.log(data);
            }
        }
        catch(err){
            console.error('getFreqDistro error: ', err.message);
        }
        return []
    }

    const getLatest = async ()=>{
        // Fetch the most recent reading so the UI can show live values.
        const controller = new AbortController();
        const signal = controller.signal;
        const id = setTimeout(()=>{controller.abort()},60000);
        const URL = `/api/weather/latest`;
        try {
            const response = await fetch(URL,{ method: 'GET', signal: signal });
            if (response.ok){
                const data = await response.json();
                let keys = Object.keys(data);
                if(keys.includes("status")){
                    if(data["status"] == "found" ){
                        return data["data"];
                    }
                    if(data["status"] == "failed"){
                        console.log("getLatest returned no data");
                    }
                }
            }
            else{
                const data = await response.text();
                console.log(data);
            }
        }
        catch(err){
            console.error('getLatest error: ', err.message);
        }
        return {}
    }
    

    return { 
    // EXPORTS
    getAllInRange,
    getTemperatureMMAR,
    getHumidityMMAR,
    getFreqDistro,
    getLatest
            
    }
},{ persist: true  });