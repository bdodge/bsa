
import java.net.*;
import java.io.*;
import java.util.*;

// translation table
12345678901234567890123456789
	a   b   c   d   e   f
	x   y   z
	  
 f  	k

class iTab {
	public static String[] k = new String[256], e = new String[256];
	public iTab() {
		// open dict file and read tokens
		try {
			FileReader inr = new FileReader("furbdict.txt");
			StreamTokenizer in = new StreamTokenizer(inr);
			String key= "";
			int j = 0;
			boolean sequence = false;
			while(in.nextToken() != StreamTokenizer.TT_EOF) {
				if(in.ttype == StreamTokenizer.TT_WORD) {
					if(sequence) { 
						k[j] = key;
						e[j] = in.sval;
						//System.out.println("adding " + key + " = " + in.sval);
						j++;
						sequence = false;
					} else {
						key = in.sval;
						sequence = true;
					}
				}
				if(j >= 255) break;
			}
			while(j < 256) {
				k[j] = null; e[j] = null;
				j++;
			}
		} catch(IOException e) {
			System.out.println("Init file furbdict.txt read error");
		}
	}
}

// generic translation service thread

class XlateThread extends Thread {
	static iTab itab = null;
	static int serverNum = 0;
	int myServerNum;
	Hashtable table = null; 
	Socket sSock = null;
	
	public XlateThread(Socket Sock) {
		super("FurbServ");
		myServerNum = ++serverNum;
		sSock = Sock;
		table = new Hashtable(256);
		if(itab == null) {
			//System.out.println("made table for server " + myServerNum);
			itab = new iTab();
		}
		initTable();
	}
	public void signon() {
		System.out.println("Starting xlation server " + myServerNum);
	}
	public void signoff() {
		System.out.println("Server " + myServerNum + " Ended");
	}
	public void initTable() {
		;
	}
	public String translate(String tok) {
		Enumeration keylist = table.keys();
		if(table.containsKey(tok)) {
			System.out.println("translation: " + tok + " => " + table.get(tok));
			return table.get(tok).toString() + "\r\n";
		} else {
			System.out.println("no translation for " + tok);
			return "No Translation for " + tok + "\r\n";
		}
	}
	public void run() {
		signon();
		if(sSock != null) {
			try {
				InputStream in = sSock.getInputStream();
				OutputStream out = sSock.getOutputStream();
				String token = "", result;
				int ic;
				while((ic = in.read()) > 0) {
					//System.out.write(ic);
					//System.out.println("["+new Integer(ic).toString()+"]");
					if(ic == ' ' || ic == '\n' || ic ==',' || ic == '\r' || ic == '\t') {
						if(token.length() > 0) {
							result = translate(token);
							if(result.length() > 0) {
								out.write(result.getBytes());
							}
						}
						token = "";
					} else {
						token = token + (char)ic;
					}
				}
				in.close();
			} catch(IOException e) {
				;
			}
		}
		signoff();
	}
}

// e->f class

class FurbServThread extends XlateThread {
	public FurbServThread(Socket sock) { super(sock); }
	public void signon() {
		System.out.print("English->Furbish ");
		super.signon();
	}
	public void initTable()
	{
		int j;
		for(j = 0; j < 256; j++) {
			if(itab.k[j] != null && itab.e[j] != null) {
				table.put(itab.k[j], itab.e[j]);
			} else {
				break;
			}
		}
	}
}

// f->e class

class EngServThread extends XlateThread {
	public EngServThread(Socket sock) { super(sock); }
	public void signon() {
		System.out.print("Furbish->English ");
		super.signon();
	}
	public void initTable()
	{
		int j;
		for(j = 0; j < 256; j++) {
			if(itab.k[j] != null) {
				table.put(itab.e[j], itab.k[j]);
			} else {
				break;
			}
		}
	}
}

// generic server class, used to start threads
// per accepted connection

class FurbyServy extends Thread {   
	ServerSocket server = null;
	int port;
	
	public FurbyServy(int portno) {
		super("FurbyServy:" + portno);
		System.out.println("Creating FurbyServy for port " + portno);
		port = portno;
	}
	public void run() {
		try {
			server = new ServerSocket(port);
		} catch(IOException e) {
			System.exit(-1);
		}
		Socket servSock = null;
		// accept connections on the port and start a thread
		// for those connections
		while(1==1) {
			try {
				servSock = server.accept();
			} catch(IOException e) {
				System.exit(-1);
			}
			if(port == 0xBEF)
				new FurbServThread(servSock).start();
			else
				new EngServThread(servSock).start();
		}
	}
}

// start a Fubish->English thread and a
//  	   English->Furbish thread
//
class FurbServer {
	public static void main(String[] args) {
		new FurbyServy(0xBEF).start(); // eng->furb 
		new FurbyServy(0xBFE).start(); // furb->eng
	} 
}

