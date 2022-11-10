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

public class Day {    
                     
    //Collections.synchronizedList(new ArrayList<YourClassNameHere>())                               // calendar invite then calendar is a
    List<Meeting> daySchedule; // = Collections.synchronizedList(new ArrayList<Meeting>());
    //ArrayList<Meeting> daySchedule;
    //LocalDate dayOfWeek;

    public Day() {
        this.daySchedule = Collections.synchronizedList(new ArrayList<Meeting>());
    }

    public List<Meeting> getDay() {
        return this.daySchedule;
    }

    public void addMeeting(Meeting meeting) {
        this.daySchedule.add(meeting);
    }


}
