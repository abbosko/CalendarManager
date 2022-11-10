package edu.cs300;

import java.time.LocalDateTime;
import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.util.*;
import java.time.*;
import java.io.FileNotFoundException;
import edu.cs300.Day;
import edu.cs300.Meeting;
import java.util.concurrent.ConcurrentHashMap;

public class Calendar {                                                    // calendar invite then calendar is a
    ConcurrentHashMap<LocalDate,Day> calendar;

    public Calendar(){
        this.calendar = new ConcurrentHashMap<LocalDate,Day>();
    }

    public ConcurrentHashMap<LocalDate,Day> getCalendar() {
        return this.calendar;
    }

    public void addToCalendar(String[] meetingLine){
        Meeting meeting = new Meeting(meetingLine);

        if (!calendar.containsKey(meeting.getDay())) {
            Day d = new Day();
            d.addMeeting(meeting);
            calendar.put(meeting.getDay(), d);
        }
        else {
            Day day = calendar.get(meeting.getDay());
            day.addMeeting(meeting);
        }
    }

    public int checkCalendar(MeetingRequest mtgReq){
        String[] meetingRequest = new String[4];
        meetingRequest[0] = mtgReq.description;
        meetingRequest[1] = mtgReq.location;
        meetingRequest[2] = mtgReq.datetime;
        meetingRequest[3] = Integer.toString(mtgReq.duration);
        int avaliable = 1;

        Meeting newMeeting = new Meeting(meetingRequest);
    
        /* 
        if( (mtgReq.request_id) ==0  ){
            System.exit(0);
        } */

        
        
        if (!calendar.containsKey(newMeeting.getDay())) {
             avaliable = 1;
             addToCalendar(meetingRequest);
         }
         else {
        
            Day day = calendar.get(newMeeting.getDay());
           // Day currentMeetings;
           // currentMeetings = calendar.get(newMeeting.getDay());

            List<Meeting> schedule = Collections.synchronizedList(day.getDay());
               
            synchronized(schedule){
            for(int i =0; i< schedule.size(); i++) {
            if (schedule.get(i).getStartTime().isBefore(newMeeting.getEndTime()) && newMeeting.getStartTime().isBefore(schedule.get(i).getEndTime())){
             //if (!schedule.get(i).getEndTime().isBefore(newMeeting.getStartTime()) && !schedule.get(i).getStartTime().isAfter(newMeeting.getEndTime())){
                avaliable = 0; 
                return 0;
            }
        }
        }
        } 

        if (avaliable ==1) { addToCalendar(meetingRequest); }
        return avaliable;
    }

   // return !t1.end.isBefore(t2.begin) && !t1.begin.isAfter(t2.end);
    }
