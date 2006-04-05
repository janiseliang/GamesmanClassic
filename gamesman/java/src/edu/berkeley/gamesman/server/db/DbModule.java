//bugs: gzip reading is broken at 2134
//this gives 772; should be 1026
//caused by byte alignment being broken
//broken at least at 1600
//help: I do not understand why this is happening!

package edu.berkeley.gamesman.server.db;

import edu.berkeley.gamesman.server.IModule;
import edu.berkeley.gamesman.server.IModuleRequest;
import edu.berkeley.gamesman.server.IModuleResponse;
import edu.berkeley.gamesman.server.ModuleException;
import edu.berkeley.gamesman.server.ModuleInitializationException;

import java.io.DataInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.SortedMap;
import java.util.TreeMap;
import java.util.zip.GZIPInputStream;

public class DbModule implements IModule
{

	private HashMap loadedData; // hash of loaded data (maps to shorts!)
	private String baseDir; //base directory of database files
	private static String MYCMD = "getValueOfMove"; //cmd I recognize
    private short FILEVER = 1; //db file version
	// private HashMap versions; //game versions table

	/* Because java sucks with data type conversion */
	/* Assumes p_src is Big Endian (network byte order) */
	/* This is slow :( */
	public static short makeShort( byte[] p_src)
	{
		/*
		 * if (p_src.length != 2){ //ERROR return 0; }
		 */
		short ret=0;		
		for (byte i=0;i<2;i++){ // compiler better unroll this
			ret <<= 8;
			ret |= (((char)p_src[i])&0xFF);
			
		}
		return ret;
	}

	/*generate int (which numpositions is stored as()*/
	/*Note: using broken db format!*/
	public static int makeInt( byte[] p_src)
	{
		/*
		 * if (p_src.length != 2){ //ERROR return 0; }
		 */
		int ret=0;		
		for (byte i=0;i<4;i++){ // compiler better unroll this
			ret <<= 8;
			ret = ret | (((char)p_src[i])&0xFF);
			System.out.println("p_src " + i + " = " + p_src[i] + "ret is " + ret);
		}
		return ret;
	}
	/*positions passed as 64 bit*/
	public static long makeLong(byte[] p_src)	{
		/*
		 * if (p_src.length != 2){ //ERROR return 0; }
		 */
		long ret=0;
		// assert(p_src.length == 8); //exception
		for (byte i=0;i<8;i++){ // compiler better unroll this
			ret = ret<< 8;
			ret = ret | (((char)p_src[i])&0xFF);
			System.out.println("p_src " + i + " = " + p_src[i] + "ret is " + ret);
		}
		return ret;
	}
			
	 public void initialize(String[] configArgs) throws ModuleInitializationException{
		 //arg[0]= directory path
		 //arg[1] = size of array of strings to preload
		 //arg[2...n] = strings to preload
		 
		 	File f; 
			FileInputStream fis; 
			GZIPInputStream gz;
			baseDir = configArgs[0] + "/"; //implicit
			int len = Integer.decode(configArgs[1]).intValue();
			short[] buf=null; // injected memory buffer
			byte[] sbuf = new byte[2]; // memory buffer for reading in short
			short s;
			byte[] lbuf = new byte[8]; // memory buffer for reading in long			
			byte[] ibuf = new byte[4]; // memory buffer for reading in int
			int num_pos=0; // this really should be long; let's hope we don't have
							// 16 gb databases anytime soon
			//if (pl.length!=0)
			//always load
			loadedData = new HashMap();
			len+=2;//account for full length
			if (len>configArgs.length)
				throw new ModuleInitializationException("corrupted arguments");
			
			for (int i = 2;i<len;i++){
				try {
				f = new File(baseDir + configArgs[i]);
				if (!f.exists()){
					System.out.println("File does not exist: " + baseDir + configArgs[i]);
					continue;
				}
					
					
				fis = new FileInputStream(f);				
				gz = new GZIPInputStream(fis);
				}
				catch (Exception e){
						//just log error and continue
					continue;
				}

				try{
				// available can be used to read stuff
				if (gz.read(sbuf, 0, 2) == -1) // error log
					continue;
				s = makeShort(sbuf);
				
				if (s!=FILEVER){
					//error and continue;
					continue;
				}
								
				if (gz.read(ibuf, 0, ibuf.length) == -1) // error log					
					continue;
				
				// FIXME: I need a version number here!!
				// actually we are ignoring this for now
				//num_pos = (int)makeLong(lbuf); // this is the size (why this is long i don't know)
				num_pos = makeInt(ibuf);
				
				//only supports reading 32 bit files - who cares
				System.out.println("num pos is " + num_pos);
				gz.skip(4); //4 bytes of 0's in broken format
				buf = new short[num_pos];
				// now load in the entire system
				for (int j=0;j<num_pos;j++){
					if (gz.read(sbuf, 0, 2) == -1){ // error log
						System.out.println("gzip file wasn't large enough");
						break;			
					}
					s = makeShort(sbuf); // this is the position
					//if (j>=13400&&j<13410)
					if (j>=2130&&j<2140){
					
						System.out.println(j + " pos is " + s);
						System.out.println("high byte is " + sbuf[0] +
								"low is " + sbuf[1]);
					}
					buf[j]=s;
				}
				}
				
				catch (IOException e){
					//throw module exception
					continue;
				}
				//check if can save!
				loadedData.put(configArgs[i], buf); // associate file with datastream
			}
	 }
	
	public DbModule () throws ModuleException{ 
		//this doesn't do anything
	}
	public boolean typeSupported(String requestTypeName){
		return requestTypeName.equalsIgnoreCase(MYCMD);
	}

	// our request goes here
	// must be mycmd as that is the only cmd supported
    public void handleRequest(IModuleRequest req, IModuleResponse res) throws ModuleException{
    	String hlen = req.getHeader("length");
    	String gamename = req.getHeader("game");
    	String gameop = req.getHeader("variation");
    	if (hlen == null || gameop == null || gamename == null){
    		// throw module error
    		return;
    	}
    	short len = Short.decode(hlen).shortValue(); // the length of request 
    	//note: using short as we don't want to kill server w/ endless requests!
    	if (len<0){
    		// malformed request error
    		return;
    	}
    	System.out.println("len is " + len);
    	if (len == 0) // just return
    		return;
    	// generate filename
    	System.out.println("dbg: gamename is " + gamename);
    	String fname = "m" + gamename + "_" + gameop + "_memdb.dat.gz";
    	System.out.println("dbg: fname is " + fname);
    	if (loadedData.containsKey(fname)){ // key logic
    		System.out.println("key entered");
    		short [] htable = (short[])loadedData.get(fname);
    		byte[] lbuf = new byte[8]; // read buffer from input stream
 		    		   		
    		try{
    		DataInputStream w = new DataInputStream(req.getInputStream());
    		for (int i = 0;i<len;i++){
    			//if (req.getInputStream().read(lbuf, 0, 8)==-1){ // must be aligned															
    				//module exception
    			//}
    			long l = w.readLong(); //64 bit, though we can only use 32
    			System.out.println("accessing " + (int)l);
    			//int l = (int)makeLong(lbuf); //only support 32 bit
    			short s = htable[(int)l];    			
    			//write in Big Endian to stream
    			System.out.println("Got " + s);
    			res.getOutputStream().write((s>>8)&0xFF);
    			res.getOutputStream().write(s&0xFF);
    		}
    		}
    		catch (IOException e){
    			//throw module exception
    		}
    		//done
    	}
    	
    	else{ // GZIP seek logic
    		System.out.println("using gzip seek");
    		GZIPInputStream gz;
    		try {
			File f = new File(baseDir + fname);
			FileInputStream fis = new FileInputStream(f);
			gz = new GZIPInputStream(fis);
		
			//collect hash of data: can handle 64 bits
			long[] inbuf = new long[len];
			byte[] lbuf = new byte[8]; // read buffer from input stream
			byte[] sbuf = new byte[2];
			// check file version
			if (gz.read(sbuf, 0, 2) == -1){ // error log
				//throw module exception
				return;
			}
			short s = makeShort(sbuf);			
			
			if (s!=FILEVER){
				//module exception
				return;
			}
			//gz.read(lbuf,0,8); //ignore this!
			gz.skip(8); //ignore size (probably should check this!)
			//build list
			for (int i = 0;i<len;i++){
				if (req.getInputStream().read(lbuf, 0, 8)==-1){ // must be aligned															
					//module exception
				}
				long l = makeLong(lbuf); //only support 32 bit
				inbuf[i] = l;
			}
			//sort it
			//long[] insort = new long[inbuf.length];
			SortedMap insort = new TreeMap();
			//insort.
			//This is why I hate java:			
			for (int q = 0;q<inbuf.length;q++)
				insort.put(new Long(inbuf[q]), new Integer(q)); //sorted logic			
			short [] outbuf = new short[inbuf.length];
			//again: only 32 bit (long limits)
			long lastpos = 0;
			//subclass this crap: and do comparable stuff to sort!
			//yay!
			
			//we iterate in ascending order to grab correct data!
			for (Iterator p=insort.entrySet().iterator(); p.hasNext();){
				Map.Entry v = (Map.Entry)p.next();
				long vn = ((Long)(v.getKey())).longValue();
				System.out.println("seeking by " + (2*vn-lastpos));
				gz.skip(2*vn-lastpos); //short array
				lastpos = 2*vn+2;
							//wiki this stuff
				if (gz.read(sbuf, 0, 2) == -1) // error log
					break;
				
				s = makeShort(sbuf); // this is the position
				
				//use value as idx to store!
				outbuf[((Integer)(v.getValue())).intValue()]=s;	
				System.out.println("outbuf["+((Integer)(v.getValue())).intValue()+"]="+s);
			}
			//dump out to stream (correctly sorted!)
			for (int i=0;i<outbuf.length;i++){
//				write in Big Endian to stream
    			res.getOutputStream().write(outbuf[i]>>8);
    			res.getOutputStream().write(outbuf[i]&0xFF);
			}
			gz.close();
    	//fixme: gzip is fux0red
    		}
			catch (Exception e){
					//throw module exception (who needs details?)
				System.out.println("Some exception fired");
				return;
			}
			//done!
			
    	}
    	// add header to response
    	res.setHeader("length", hlen); // client gets back what it sent
    }
    	
    	
}

