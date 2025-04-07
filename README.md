## CRM System for Shift and Work Session Management

This is a desktop CRM application built with **C++** and the **Qt Framework** designed to efficiently manage work shifts, employee activity, and client interactions. The system is tailored to two primary user roles: **Admin** and **Employee**. It enables seamless **shift scheduling**, **real-time shift monitoring**, **report generation**, and **screenshot logging**.

---

### Key Features

#### **Admin Role:**
- **Create and Assign Shifts**: Admins can create shifts and assign them to employees. They can specify:
  - **Shift Time**
  - **Client**
  - **Assigned Employee**
  - **Shift Notes**
  
- **Monitor Active Shifts**: Admins can view ongoing shifts in real-time, ensuring that employees are staying on task and that clients are being served properly.

- **Generate Shift Reports**: After a shift is completed, Admins can view detailed reports that include:
  - Shift duration
  - Employee and client information
  - Screenshots before and after the shift

#### **Employee Role:**
- **View Assigned Shifts**: Employees can easily check their upcoming shifts and view the shift details, including client information, shift time, and assigned notes.

- **Start and Finish Shifts**: Employees can start a shift when the scheduled time arrives. Upon completing the shift, they can mark it as finished, which will trigger the report creation process.

- **Add Screenshots**: Employees can upload screenshots before, during, or after the shift, documenting their work session. This is useful for verification and quality control.

- **Submit Final Shift Reports**: Once the shift is completed, the employee submits a final report that includes all relevant data and screenshots. This helps in compiling data for performance and client analysis.

---

### Technologies Used
- **C++** and **Qt Framework** for building the desktop application.
- **SQLite** for lightweight database storage of shift and employee data.
- **Qt Widgets/UI Forms** for creating the user interface, providing an intuitive experience for both admins and employees.

---

### Screenshots


![Admin Dashboard](https://github.com/user-attachments/assets/bf49c3d9-4c10-4dba-a9d9-ea68ecdb28ca)


![Employee Shift View](https://github.com/user-attachments/assets/497a35c4-31a3-4ed2-8f6a-4b2ef6d3e9e0)


![Shift Start Screen](https://github.com/user-attachments/assets/04ca20db-1f33-424e-940c-271ecf7f519e)


![Shift Completed](https://github.com/user-attachments/assets/24bd48c0-082f-49c1-9f8a-98823b1aeb24)

![Employee Screenshot Before Shift](https://github.com/user-attachments/assets/9f69ef22-4798-448d-a67f-e1873dab1fee)

![Employee Screenshot After Shift](https://github.com/user-attachments/assets/056f2dd1-4fa1-4fa5-b92d-44388ff20dfe)



### Installation

1. Clone this repository to your local machine:
   ```bash
   git clone https://github.com/ktalovskih/customerRelationshipManagement.git
   ```

2. **Build the project** using C++ and Qt:
   - Ensure you have the Qt development environment set up on your system.
   - Open the project in Qt Creator and build the application.

3. **Run the application**:
   - Launch the desktop app to start managing shifts and work sessions.

---

### Future Enhancements
- **Real-Time Notifications**: Notify employees of upcoming shifts or when shifts are about to end.
- **Employee Scheduling**: Introduce a calendar view for admins to better plan and organize shifts.
- **Client Feedback System**: Allow clients to provide feedback on employee performance.
- **Detailed Analytics**: Add functionality for admins to generate reports based on performance, shift data, and client satisfaction.

