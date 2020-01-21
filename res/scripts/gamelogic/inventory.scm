(use-modules (srfi srfi-9))

(define-record-type <attribute>
  (make-attribute name min_value max_value value on-minimum on-maximum on-change)
  attribute?
  (name attribute-name attribute-name!)
  (min_value attribute-min-value)
  (max_value attribute-max-value)
  (value attribute-value attribute-value!)
  (on-minimum attribute-on-minimum)
  (on-maximum attribute-on-maximum)
  (on-change  attribute-on-change)
)

(define* (attribute attribute_name min_value max_value #:key (on-min noop) (on-max 2) (on-change 2))
  (make-attribute attribute_name min_value max_value 0 on-min on-max on-change)
) 

(define (make-attributes . attributes)
  (define attribute_map (make-hash-table))
  (for-each (lambda (attribute) (hash-set! attribute_map (attribute-name attribute) attribute)) attributes)
  attribute_map
)
(define (set-value attributeMap name value) 
  (define attribute (hash-ref attributeMap name))
  (define oldvalue (attribute-value attribute))
  (attribute-value! attribute (min (max value 0) (attribute-max-value attribute))) 
  (if (<= value (attribute-min-value attribute)) ((attribute-on-minimum attribute)))
  (if (>= value (attribute-max-value attribute)) ((attribute-on-maximum attribute)))
  ((attribute-on-change attribute) value)
)
(define (get-value attributeMap name) (attribute-value (hash-ref attributeMap name)))
(define (delta-value attributeMap name deltaValue) (set-value attributeMap name (+ (get-value attributeMap name) deltaValue)))

#! EXAMPLE USAGE
(define (onlow) 
  (display (string-append "on low: " "health: " (number->string (get-value stats "health")) "\n"))
)
(define (onmax)
  (display (string-append "on max: " "health: " (number->string (get-value stats "health")) "\n"))
)

(define (onchange newvalue)
  (if (= newvalue 40)
    (display (string-append "on change 40: " "health: " (number->string (get-value stats "health")) "\n"))
  )
)

(define stats 
  (make-attributes
    (attribute "health" 0 100 #:on-min onlow)
    (attribute "mana" 0 20 #:on-min onlow)
  )
)

(define stats
  (make-attributes 
    (attribute "health" 0 100 #:on-min onlow #:on-max onmax #:on-change onchange)
  )
)

(set-value stats "health" 50)
(display (string-append "health: " (number->string (get-value stats "health")) "\n"))

(delta-value stats "health" -10)
(display (string-append "health: " (number->string (get-value stats "health")) "\n"))

(delta-value stats "health" -10)
(display (string-append "health: " (number->string (get-value stats "health")) "\n"))

(set-value stats "health" 50)
(display (string-append "health: " (number->string (get-value stats "health")) "\n"))

(delta-value stats "health" -50)
(display (string-append "health: " (number->string (get-value stats "health")) "\n"))

(set-value stats "health" 120)
!#
;;;;;;;;;;;;;;;;;;;;;