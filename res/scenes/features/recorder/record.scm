
(define activeRecordingId -1)

(define (playrecording)
  (define id (play-recording (gameobj-id (lsobj-name "box")) "./res/recordings/record.rec"))
  (set! activeRecordingId id)
)
(define (stoprecording)
  (stop-recording activeRecordingId "./res/recordings/record.rec")
  (set! activeRecordingId -1)
)

(define recordingId -1)
(define (createrecording)
  (define id (create-recording (gameobj-id (lsobj-name "box"))))
  (set! recordingId id)
)
(define (saverecording)
  (save-recording recordingId "./res/recordings/record.rec")
  (set! recordingId -1)
)

(define (onKeyChar key)
  (cond 
    ((= key 46) (playrecording))     ; .
    ((= key 47) (stoprecording))     ; /
    ((= key 59) (createrecording))   ; ;
    ((= key 39) (saverecording))     ; '
  )
  (display (string-append "key is: " (number->string key) "\n"))
)


