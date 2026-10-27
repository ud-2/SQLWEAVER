-- ============================================================================
-- SQLWeaver Demo 2: Security & Quality Gate Violations
-- Purpose: Verifies automated CI/CD gating and static linter rules.
-- Expected Linter Outcome: FAIL (Exit code 2, Multiple Violations)
-- ============================================================================

CREATE OR REPLACE PROCEDURE security_audit_demo (
    p_user_input IN VARCHAR2,
    p_emp_id     IN NUMBER
) IS
    v_sql VARCHAR2(1000);
BEGIN
    -- Violation PERF-001: SELECT * is discouraged in production code
    FOR rec IN (SELECT * FROM hr.employees) LOOP
        DBMS_OUTPUT.PUT_LINE(rec.first_name);
    END LOOP;

    -- Violation SEC-001: Critical UPDATE without WHERE clause (Modifies all rows!)
    UPDATE hr.employees
       SET salary = 50000;

    -- Violation SEC-002: Critical DELETE without WHERE clause (Wipes table!)
    DELETE FROM hr.departments;

    -- Violation SEC-003: Critical SQL Injection via Dynamic SQL concatenation
    v_sql := 'DELETE FROM hr.employees WHERE last_name = ''' || p_user_input || '''';
    EXECUTE IMMEDIATE v_sql;

EXCEPTION
    -- Violation QUAL-001: Silent exception swallowing (Anti-pattern)
    WHEN OTHERS THEN
        NULL;
END security_audit_demo;
/