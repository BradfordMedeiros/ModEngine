(define (calcValue min max percentage)
  (define range (- max min))
  (define ratio (+ min (* percentage range)))
  ratio
)
(define (onMessage key value)
  (define attr (gameobj-attr (gameobj-by-id (string->number value))))
  (define min (cadr (assoc "min" attr)))
  (define max (cadr (assoc "max" attr)))
  (define percentage (cadr (assoc "slideamount" attr)))
  (define newValue (calcValue min max percentage))
  (format #t "min = ~a, max = ~a, percentage = ~a, value = ~a\n" min max percentage newValue)

)