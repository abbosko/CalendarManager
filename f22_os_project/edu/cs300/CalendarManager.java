package edu.cs300;
import java.io.File;
import java.io.FileNotFoundException;
import java.util.Hashtable;
import java.util.Scanner;
import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.atomic.AtomicInteger;

import javax.naming.ldap.LdapName;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.util.concurrent.locks.Condition;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;



public class CalendarManager {

	Hashtable<String,ArrayBlockingQueue<MeetingRequest>> empQueueMap;
	ArrayBlockingQueue<MeetingResponse> resultsOutputArray;


	public CalendarManager() {
		this.resultsOutputArray = new ArrayBlockingQueue<MeetingResponse>(30);
		empQueueMap = new Hashtable<String,ArrayBlockingQueue<MeetingRequest>>();
		ArrayBlockingQueue<MeetingRequest> empIncomingRequests = new  ArrayBlockingQueue<MeetingRequest>(10);

		String[] employee;
		String line;
		try {
			BufferedReader br = new BufferedReader(new FileReader("employees.csv"));
			while ((line = br.readLine()) != null) {
				employee = line.split(",");
				empQueueMap.put(employee[0], empIncomingRequests);
				new Worker(employee[0],empIncomingRequests, this.resultsOutputArray, employee[1]).start();
			}
		}
		catch (FileNotFoundException ex) { ex.printStackTrace(); }
		catch (IOException e) { e.printStackTrace(); }

	
		new OutputQueueProcessor(this.resultsOutputArray).start();
		new InputQueueProcessor(this.empQueueMap).start();


	}

	public static void main(String args[]) {
	
				CalendarManager mgr = new CalendarManager();
		
	}

	class OutputQueueProcessor extends Thread {

		ArrayBlockingQueue<MeetingResponse> resultsOutputArray;

		OutputQueueProcessor(ArrayBlockingQueue<MeetingResponse> resultsOutputArray){
			this.resultsOutputArray=resultsOutputArray;
		}

		public void run() {
			DebugLog.log(getName()+" processing responses ");
			while (true) {
				try {
					MeetingResponse res = resultsOutputArray.take();
					MessageJNI.writeMtgReqResponse(res.request_id, res.avail);
					System.out.println(getName()+" writing response "+res.request_id + " with avaliability : " + res.avail);

				} catch (Exception e) {
					System.out.println("Sys5OutputQueueProcessor error "+e.getMessage());
				}

			}

		}

	}

	class InputQueueProcessor extends Thread {
		Hashtable<String,ArrayBlockingQueue<MeetingRequest>> empQueueMap;
		
		InputQueueProcessor(Hashtable<String,ArrayBlockingQueue<MeetingRequest>> empQueueMap){
			this.empQueueMap=empQueueMap;
		}

		public void run(){
			while (true) {
				MeetingRequest req = MessageJNI.readMeetingRequest();
				if (req.isEndMsg()){
					// end thread 
				}
				try {
					if (empQueueMap.containsKey(req.empId)) {
						System.out.println(getName()+" recvd msg from queue for ");
						empQueueMap.get(req.empId).put(req);
						DebugLog.log(getName()+" pushing req "+req+" to "+req.empId);
					
					}

				} catch (InterruptedException e) {
					DebugLog.log(getName()+" Error putting to emp queue"+req.empId);
					e.printStackTrace();
				}
			}
		}

	}

}
