package edu.cs300;

import java.util.concurrent.ArrayBlockingQueue;
import edu.cs300.Meeting;
import java.io.BufferedReader;
import java.io.FileReader;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.*;
import java.io.*;
import java.io.FileWriter;

class Worker extends Thread{

	ArrayBlockingQueue<MeetingRequest> incomingRequests;
	ArrayBlockingQueue<MeetingResponse> outgoingResponse;
	String empId;
	String employeeFile;
	//final Lock lock;
	//final Condition notEmpty; 


	public Worker(String empId,ArrayBlockingQueue<MeetingRequest> incomingRequests, ArrayBlockingQueue<MeetingResponse> outgoingResponse, String employeeFile){
		this.incomingRequests=incomingRequests;
		this.outgoingResponse=outgoingResponse;
		this.empId=empId;
		this.employeeFile = employeeFile;
	}

	public void run() {
		DebugLog.log(" Thread ("+this.empId+") thread started ...");
		try {
			String line = "";
			String meetingLine[];
			int avail;
			Calendar myCalendar = new Calendar();
			BufferedReader br = new BufferedReader(new FileReader(this.employeeFile));

			while ((line = br.readLine()) != null) {
				line = line.replace("\"", "");
				meetingLine =  line.split(",");
				myCalendar.addToCalendar(meetingLine);
			}

			while(true){
			MeetingRequest mtgReq=(MeetingRequest)this.incomingRequests.take();
			/*if (mtgReq.isEndMsg()) {
			
					Writer filewrite = new FileWriter(this.empId + ".bak", false);
					filewrite.write(myCalendar.getCalendar().toString());
					avail = 0;
			} */
			//else {
			avail = myCalendar.checkCalendar(mtgReq); //}
			DebugLog.log("Worker-" + this.empId + " " + mtgReq+" pushing response" +mtgReq.request_id );
			this.outgoingResponse.put(new MeetingResponse(mtgReq.request_id, avail));
			}
		}

		catch (FileNotFoundException ex) { ex.printStackTrace(); }
		catch (IOException e) { e.printStackTrace(); }
		catch(InterruptedException e){
			System.err.println(e.getMessage());
		}

	}

}
