package edu.cs300;

import java.time.LocalDateTime;
import java.util.Calendar;
import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Hashtable;
import java.util.*;
import java.time.*;
import java.io.FileNotFoundException;

public class Meeting {                                                    // calendar invite then calendar is a
    String description;
    String location;
    LocalDateTime datetime;
    LocalDate date;

    int duration;

    public Meeting(String[] input){
        this.description = setDescription(input);
        this.location = setLocation(input);
        this.datetime = setDatetime(input);
        this.date = setDate(datetime);
        this.duration = setDuration(input);
    }

    public LocalDateTime getStartTime(){
        return this.datetime;
    }

    public LocalDateTime getEndTime(){
        return this.datetime.plusMinutes(this.duration);
    }

    public String setDescription(String[] input)
    {
        return input[0];
    }

    public String getDescription()
    {
        return this.description;
    }

    public String setLocation(String[] input){
        return input[1];
    }

    public String getLocation(){
        return this.location;
    }

    public LocalDateTime setDatetime(String[] input){
        return LocalDateTime.parse(input[2]);
    }

    public LocalDate setDate(LocalDateTime datetime){
        return datetime.toLocalDate();
    }

    public int setDuration(String[] input) {
        return Integer.parseInt(input[3]);
    }

    public LocalDate getDay() {
        return this.date;
    }

    // public String getTime(){
    //     Instant instant = localDateTime.atZone(ZoneId.systemDefault()).toInstant();
    //     return Time.from(instant);
    // }


    // public void populateCalendar( String empId) {
    //     String line = "";
    //     String meeting[];

    //     try {
    //     BufferedReader br = new BufferedReader(new FileReader(this.employeeFile));

    //     while ((line = br.readLine()) != null) {
    //         meeting =  line.split(",");
    //         parseMeeting(meeting);
    //         //weekDay.put(meeting)
    //         //addMeeting(meeting);

    //     }
    //     }
    // 	catch (FileNotFoundException ex) { ex.printStackTrace(); }
    //     catch (IOException e) { e.printStackTrace(); }



    //     }

    // public void addMeeting(String[] meeting) {
    //     weekDay.add(parseMeeting(meeting));
    //     empCalendar.put(getDay(meeting), weekDay);
    // }

    // public String getDay(String[] line) {
    //     LocalDateTime dt = LocalDateTime.parse(line[2]);
    //     //String dayOfWeek = dt.getDayofWeek();
    //     return dt.getDayOfWeek().name();

    // }

    // public Hashtable parseMeeting(String[] meeting){
    //     // "staff meeting","conference room",2022-12-20T15:30,60

    //     this.schedule.put("description", meeting[0]);
    //     this.schedule.put("location", meeting[1]);
    //     this.schedule.put("datetime", meeting[2]);
    //     this.schedule.put("duration", meeting[3]);

    //     return this.schedule;

    // }


    // public void addToCalendar(String line, String index) {
    //     String datetime = line[2];
    //     empCalendar.add(0, empId);                              // key 0 = index in array list
    //     empCalendar.add(getDay(datetime), line);

    // }



}
