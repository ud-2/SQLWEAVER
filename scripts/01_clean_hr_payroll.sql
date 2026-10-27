-- ============================================================================
-- SQLWeaver Demo 1: Clean DDL & Stored Procedure
-- Purpose: Validates clean parsing, line offsets, and accurate data lineage.
-- Expected Linter Outcome: PASS (Exit code 0, 0 Critical Violations)
-- ============================================================================

-- Block 1: DDL Table Creation
CREATE TABLE hr.departments (
    dept_id     NUMBER PRIMARY KEY,
    dept_name   VARCHAR2(100) NOT NULL,
    created_at  DATE DEFAULT SYSDATE
);
/

-- Block 2: DDL Table Creation
CREATE TABLE hr.employees (
    emp_id      NUMBER PRIMARY KEY,
    dept_id     NUMBER,
    first_name  VARCHAR2(50),
    last_name   VARCHAR2(50),
    salary      NUMBER DEFAULT 0
);
/

-- Block 3: Stored Procedure Processing Payroll
CREATE OR REPLACE PROCEDURE hr.process_department_payroll (
    p_dept_id   IN NUMBER,
    p_status    OUT VARCHAR2
) IS
    v_total_salary NUMBER := 0;
BEGIN
    -- Read from departments table
    SELECT dept_name
      INTO p_status
      FROM hr.departments
     WHERE dept_id = p_dept_id;

    -- Update employees with valid WHERE clause (Safe Write)
    UPDATE hr.employees
       SET salary = salary * 1.05
     WHERE dept_id = p_dept_id;

    COMMIT;
    p_status := 'SUCCESS';
EXCEPTION
    WHEN NO_DATA_FOUND THEN
        p_status := 'NOT_FOUND';
END process_department_payroll;
/