-- ============================================================================
-- SQLWeaver Demo 3: Complex PL/SQL Syntax & Edge Cases
-- Purpose: Verifies soft-keyword resolution (date_format), CTE filtering,
--          subquery I/O context switching, and GRANT filtering.
-- Expected Linter Outcome: PASS (Exit code 0, Accurate Lineage)
-- ============================================================================

CREATE OR REPLACE PROCEDURE billing.process_complex_invoices (
    p_date_debut   IN VARCHAR2,
    p_total_amount OUT NUMBER
) IS
    -- Soft-keyword test: date_format must parse as a regular variable identifier
    date_format VARCHAR2(20) := 'YYYY-MM-DD';
    v_bonus     NUMBER := 500;
BEGIN
    -- Subquery Factoring (CTE) test: 'summary_cte' must NOT appear as a real table
    WITH summary_cte AS (
        SELECT invoice_id, amount
          FROM billing.invoices
         WHERE status = 'PAID'
    )
    SELECT NVL(SUM(amount), 0)
      INTO p_total_amount
      FROM summary_cte;

    -- Subquery inside UPDATE test:
    -- 'billing.accounts' should be WRITE (Out)
    -- 'billing.rates' should be READ (In) despite being inside the UPDATE statement
    UPDATE billing.accounts a
       SET balance = balance + (
           SELECT rate_multiplier * v_bonus
             FROM billing.rates r
            WHERE r.rate_code = a.account_type
       )
     WHERE a.is_active = 1;

    COMMIT;
END process_complex_invoices;
/

-- Verify that GRANT statements do not extract procedures as tables
GRANT EXECUTE ON billing.process_complex_invoices TO finance_role;
/