from django.db import models
from django.utils import timezone
from datetime import datetime

class Transaction(models.Model):
    id = models.AutoField(primary_key=True)
    User = models.CharField(max_length=100, null = False)
    DateTime = models.DateTimeField("Transaction Timestamp", null = False, default=datetime.now)
    description = models.CharField(max_length=100, null = False)

    expense = models.CharField(max_length=300, null = True, default = 'Empty')
    revenue = models.CharField(max_length=300, null = True, default = 'Empty')
    asset = models.CharField(max_length=300, null = True, default = 'Empty')
    liability = models.CharField(max_length=300, null = True, default = 'Empty')
    
    def __str__(self):
        """Returns a string representation of a transaction."""
        date = timezone.now()
        return f"'{self.description}' logged on {date.strftime('%A, %d %B, %Y at %X')}"